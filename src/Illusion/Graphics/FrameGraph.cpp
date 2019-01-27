////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FrameGraph.hpp"

#include "../Core/Logger.hpp"
#include "BackedImage.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "Utils.hpp"
#include "Window.hpp"

#include <queue>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setName(std::string const& name) {
  mName = name;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setFormat(vk::Format format) {
  mFormat = format;
  mDirty  = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setSizing(ResourceSizing sizing) {
  mSizing = sizing;
  mDirty  = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setExtent(glm::uvec2 const& extent) {
  mExtent = extent;
  mDirty  = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setSamples(
    vk::SampleCountFlagBits const& samples) {
  mSamples = samples;
  mDirty   = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 FrameGraph::LogicalResource::getAbsoluteExtent(glm::uvec2 const& windowExtent) const {
  return (mSizing == ResourceSizing::eAbsolute) ? glm::uvec2(mExtent)
                                                : glm::uvec2(mExtent * glm::vec2(windowExtent));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameGraph::LogicalResource::isDepthResource() const {
  return Utils::isDepthFormat(mFormat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameGraph::LogicalResource::isColorResource() const {
  return Utils::isColorFormat(mFormat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::LogicalPass::assignResource(
    LogicalResource const& resource, ResourceAccess access) {

  try {
    assignResource(resource, access, {});
  } catch (std::runtime_error const& e) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName + "\": " + e.what());
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::LogicalPass::assignResource(
    LogicalResource const& resource, vk::ClearValue const& clear) {

  try {
    assignResource(resource, ResourceAccess::eWriteOnly, clear);
  } catch (std::runtime_error const& e) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName + "\": " + e.what());
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::LogicalPass::setName(std::string const& name) {
  mName = name;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::LogicalPass::setProcessCallback(
    std::function<void(CommandBufferPtr)> const& callback) {
  mProcessCallback = callback;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource const* FrameGraph::LogicalPass::getDepthAttachment() const {
  for (auto const& r : mLogicalResources) {
    if (r.first->isDepthResource()) {
      return r.first;
    }
  }
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::LogicalPass::assignResource(LogicalResource const& resource, ResourceAccess access,
    std::optional<vk::ClearValue> const& clear) {
  // We cannot add the same resource twice.
  if (mLogicalResources.find(&resource) != mLogicalResources.end()) {
    throw std::runtime_error("Resource has already been added to this pass!");
  }

  // We cannot add multiple depth attachments.
  if (resource.isDepthResource() && getDepthAttachment() != nullptr) {
    throw std::runtime_error("Pass already has a depth attachment!");
  }

  mLogicalResources[&resource] = {access, clear};
  mDirty                       = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::FrameGraph(
    std::string const& name, DevicePtr const& device, FrameResourceIndexPtr const& frameIndex)
    : Core::NamedObject(name)
    , mDevice(device)
    , mPerFrame(frameIndex, [device, this](uint32_t index) {
      PerFrame    perFrame;
      std::string prefix = std::to_string(index) + " of " + getName();

      perFrame.mPrimaryCommandBuffer    = CommandBuffer::create("CommandBuffer " + prefix, device);
      perFrame.mRenderFinishedSemaphore = device->createSemaphore("RenderFinished " + prefix);
      perFrame.mFrameFinishedFence      = device->createFence("FrameFinished " + prefix);
      return perFrame;
    }) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::createResource() {
  mDirty = true;
  mLogicalResources.push_back({});
  return mLogicalResources.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::createPass() {
  mDirty = true;
  mLogicalPasses.push_back({});
  return mLogicalPasses.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::setOutput(
    WindowPtr const& window, LogicalPass const& pass, LogicalResource const& resource) {
  mOutputWindow   = window;
  mOutputPass     = &pass;
  mOutputResource = &resource;
  mDirty          = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::process(uint32_t threadCount) {

  // -------------------------------------------------------------------------------------------- //
  // ---------------------------------- graph validation phase ---------------------------------- //
  // -------------------------------------------------------------------------------------------- //

  // The FrameGraph is dirty when a LogicalPass or LogicalResource was added, when a LogicalResource
  // was added to one of its LogicalPasses or when one of its LogicalResources was reconfigured.
  if (isDirty()) {

    // First make some sanity checks. After this call we can be sure that there is exactly one
    // LogicalPass with an output window and that all references to LogicalResources in the
    // LogicalPasses are valid.
    try {
      validate();
    } catch (std::runtime_error e) {
      throw std::runtime_error("Frame graph validation failed: " + std::string(e.what()));
    }

    // Then make sure that we re-create all physical passes and physical resources.
    for (auto& perFrame : mPerFrame) {
      perFrame.mDirty = true;
    }

    // Reset dirty flags of the FrameGraph, the LogicalPasses and the LogicalResources.
    clearDirty();
  }

  // -------------------------------------------------------------------------------------------- //
  // ------------------------------- resource allocation phase ---------------------------------- //
  // -------------------------------------------------------------------------------------------- //

  // Acquire our current set of per-frame resources.
  auto& perFrame = mPerFrame.current();

  // Make sure that the GPU has finished processing the last frame which has been rendered with this
  // set of per-frame resources.
  mDevice->waitForFence(perFrame.mFrameFinishedFence);
  mDevice->resetFence(perFrame.mFrameFinishedFence);

  // The PhysicalPasses and PhysicalResources need to be updated. This could definitely be optimized
  // with more fine-grained dirty flags, but as this should not happen on a frame-to-frame basis, it
  // seems to be ok to recreate everything from scratch here. To give an overview of the code below,
  // here is a rough outline:
  // * Create a list of PhysicalPasses, one for each reachable LogicalPass
  //   * Find the final output pass.
  //   * Recursively add required input passes to the list.
  //   * Reverse the list
  // * Merge adjacent PhysicalPasses which can be executed as sub passes
  // * Create a BackedImage for each PhysicalResource
  // * Create a RenderPass for each PhysicalPass
  // * Create a FrameBuffer for each RenderPass
  if (perFrame.mDirty) {
    Core::Logger::debug() << "Constructing frame graph..." << std::endl;

    // Compute logical pass execution order --------------------------------------------------------

    perFrame.mPhysicalPasses.clear();

    // First we will create a list of PhysicalPasses with a valid execution order. This list may
    // contain less passes than this FrameGraph has LogicalPasses, as some passes may not be
    // connected to our final pass (pass culling). We will collect the passes bottom-up; that means
    // we start with the final output pass and then collect all passes which provide input for this
    // pass. Then we search for the passes which provide input for those passes... and so on.
    // Therefore we will create a queue (actually we use a std::list as we have to remove
    // duplicates) of inputs which are required for the processing of the passes inserted into our
    // mPhysicalPasses. We will start with our final pass.
    std::list<LogicalPass const*> passQueue;
    passQueue.push_back(mOutputPass);

    while (!passQueue.empty()) {
      // Pop the current pass from the queue.
      auto logicalPass = passQueue.front();
      passQueue.pop_front();

      // Skip passes without any resources
      if (logicalPass->mLogicalResources.size() == 0) {
        Core::Logger::debug() << "  Skipping pass \"" + logicalPass->mName +
                                     "\" because it has no resources assigned."
                              << std::endl;
        continue;
      }

      // And create a PhysicalPass for it. We store the extent of the pass for easier later access.
      // Due to the previous graph validation we are sure that all resources have the same
      // resolution.
      PhysicalPass physicalPass({{logicalPass}});
      for (auto const& resource : logicalPass->mLogicalResources) {
        physicalPass.mExtent = resource.first->getAbsoluteExtent(mOutputWindow->pExtent.get());
        break;
      }

      Core::Logger::debug() << "  Resolving dependencies of pass \"" + logicalPass->mName + "\"..."
                            << std::endl;

      // We start searching for preceding passes at our current pass.
      auto currentPass = mLogicalPasses.rbegin();
      while (&(*currentPass) != logicalPass) {
        ++currentPass;
      }

      // Now we have to find the passes which are in front of the current pass in the mLogicalPasses
      // list of the FrameGraph and write to its the resources.
      for (auto r : logicalPass->mLogicalResources) {
        Core::Logger::debug() << "    resource \"" + r.first->mName + "\"" << std::endl;

        // Step backwards through all LogicalPasses, collecting all passes writing to this resource.
        auto previousPass = currentPass;
        auto previousUse  = previousPass->mLogicalResources.find(r.first);

        while (previousPass != mLogicalPasses.rend()) {
          do {
            ++previousPass;
          } while (previousPass != mLogicalPasses.rend() &&
                   (previousUse = previousPass->mLogicalResources.find(r.first)) ==
                       previousPass->mLogicalResources.end());

          // Now there are several cases:
          // * There is a preceding use of this resource and ...
          //   * we use it as write-only: As this would discard any content, we consider this as an
          //     error for now.
          //   * there it is used as read-only: We can ignore this case as the resource won't be
          //     modified by this pass.
          //   * there it is written: The corresponding pass has to be executed before the current
          //     pass.
          // * There is no preceding use and ...
          //   * we use it as write-only: This is alright, we are "creating" the resource
          //   * we want to read from the resource. This is an error.
          if (previousPass != mLogicalPasses.rend()) {
            if (r.second.mAccess == ResourceAccess::eWriteOnly) {
              throw std::runtime_error("Frame graph construction failed: Write-only output \"" +
                                       r.first->mName + "\" of pass \"" + logicalPass->mName +
                                       "\" is used by the preceding pass \"" + previousPass->mName +
                                       "\"!");
            } else if (previousUse->second.mAccess == ResourceAccess::eReadOnly ||
                       previousUse->second.mAccess == ResourceAccess::eLoad) {
              Core::Logger::debug()
                  << "      is read-only in pass \"" + previousPass->mName + "\"." << std::endl;
            } else {
              // In order to make sure that there are no duplicates in our queue, we first remove
              // all entries referencing the same pass.
              passQueue.remove(&(*previousPass));
              passQueue.push_back(&(*previousPass));
              physicalPass.mDependencies.push_back(&(*previousPass));

              Core::Logger::debug()
                  << "      is written by pass \"" + previousPass->mName + "\"." << std::endl;
            }
          } else if (physicalPass.mDependencies.size() == 0) {
            if (r.second.mAccess == ResourceAccess::eWriteOnly) {
              Core::Logger::debug() << "      is created by this pass." << std::endl;
            } else {
              throw std::runtime_error("Frame graph construction failed: Input \"" +
                                       r.first->mName + "\" of pass \"" + logicalPass->mName +
                                       "\" is not write-only but no previous pass writes to it!");
            }
          }
        }
      }

      // Finally we can add the physicalPass to the list.
      perFrame.mPhysicalPasses.push_back(physicalPass);
    }

    // Now we have to reverse our list of passes as we collected it bottom-up.
    perFrame.mPhysicalPasses.reverse();

    // Print some debugging information.
    Core::Logger::debug() << "  Logical pass execution order will be" << std::endl;
    for (auto const& p : perFrame.mPhysicalPasses) {
      Core::Logger::debug() << "    Pass " << p.mSubPasses[0]->mName << std::endl;
    }

    // Merge adjacent PhysicalPasses which can be executed as sub passes ---------------------------

    // Now we can merge adjacent PhysicalPasses which have the same extent and share the same depth
    // attachment. To do this, we traverse the list of PhysicalPasses front-to-back and for each
    // pass we search for candidates sharing extent, depth attachment and dependencies.
    auto current = perFrame.mPhysicalPasses.begin();
    while (current != perFrame.mPhysicalPasses.end()) {

      // Keep a reference to the current depth attachment. This may be nullptr, in this case we
      // accept any depth attachment of the candidates.
      auto currentDepthAttachment = current->mSubPasses[0]->getDepthAttachment();

      // Now look for merge candidates. We start with the next pass.
      auto candidate = current;
      ++candidate;

      while (candidate != perFrame.mPhysicalPasses.end()) {
        // For each candidate, the depth attachment must be the same as the current passes depth
        // attachment. Or either may be nullptr.
        auto candidateDepthAttachment = candidate->mSubPasses[0]->getDepthAttachment();
        bool sameDepthAttachment      = currentDepthAttachment == nullptr ||
                                   candidateDepthAttachment == nullptr ||
                                   candidateDepthAttachment == currentDepthAttachment;

        // They must share the same extent.
        bool extentMatches = candidate->mExtent == current->mExtent;

        // And all dependencies of the candidate must be satisfied. That means all dependencies of
        // the candidate must either be dependencies of the current pass as well or they have to be
        // sub passes of the current pass.
        bool dependenciesSatisfied = true;
        for (auto const& dependency : candidate->mDependencies) {
          bool isDependencyOfCurrent =
              std::find(current->mDependencies.begin(), current->mDependencies.end(), dependency) !=
              current->mDependencies.end();
          bool isSubPassOfCurrent =
              std::find(current->mSubPasses.begin(), current->mSubPasses.end(), dependency) !=
              current->mSubPasses.end();
          if (!isDependencyOfCurrent && !isSubPassOfCurrent) {
            dependenciesSatisfied = false;
            break;
          }
        }

        // If all conditions are fulfilled, we can make the candidate a sub pass of the current
        // PhysicalPass. If the current depth attachment has been nullptr, we can now use the depth
        // attachment of the candidate for further candidate checks.
        if (extentMatches && sameDepthAttachment && dependenciesSatisfied) {
          current->mSubPasses.push_back(candidate->mSubPasses[0]);

          if (!currentDepthAttachment) {
            currentDepthAttachment = candidateDepthAttachment;
          }

          // Erase the candidate from the list of PhysicalPasses and look for more merge candidates
          // in the next iteration.
          candidate = perFrame.mPhysicalPasses.erase(candidate);
        } else {
          // Look for more merge candidates in the next iteration.
          ++candidate;
        }
      }

      // Look for potential merge candidates of the next PhysicalPass.
      ++current;
    }

    // Print some debugging information.
    Core::Logger::debug() << "  Physical pass execution order will be" << std::endl;
    uint32_t counter = 0;
    for (auto const& pass : perFrame.mPhysicalPasses) {
      Core::Logger::debug() << "    RenderPass " << counter++ << std::endl;

      for (auto const& subPass : pass.mSubPasses) {
        Core::Logger::debug() << "      SubPass " << subPass->mName << " " << std::endl;
      }
    }

    // Create a BackedImage for each PhysicalResource ----------------------------------------------

    // Now we will create actual PhysicalResources. We will create everything from scratch here.
    // This can definitely be optimized, but for now frequent graph changes are not planned anyways.
    perFrame.mPhysicalResources.clear();

    // First we will create a set of resources which are actually used by the passes we collected.
    // For each resource we will accumulate the vk::ImageUsageFlagBits.
    std::unordered_map<LogicalResource const*, vk::ImageUsageFlags> resources;

    for (auto const& pass : perFrame.mPhysicalPasses) {
      for (auto const& subPass : pass.mSubPasses) {
        for (auto const& resource : subPass->mLogicalResources) {
          switch (resource.second.mAccess) {
          case ResourceAccess::eReadOnly:
            resources[resource.first] |= vk::ImageUsageFlagBits::eInputAttachment;
            break;
          case ResourceAccess::eLoad:
          case ResourceAccess::eWriteOnly:
          case ResourceAccess::eLoadWrite:
            if (resource.first->isColorResource()) {
              resources[resource.first] |= vk::ImageUsageFlagBits::eColorAttachment;
            } else if (resource.first->isDepthResource()) {
              resources[resource.first] |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            }
            break;
          case ResourceAccess::eReadWrite:
          case ResourceAccess::eLoadReadWrite:
            resources[resource.first] |= vk::ImageUsageFlagBits::eInputAttachment;
            if (resource.first->isColorResource()) {
              resources[resource.first] |= vk::ImageUsageFlagBits::eColorAttachment;
            } else if (resource.first->isDepthResource()) {
              resources[resource.first] |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            }
            break;
          }
        }
      }
    }

    // For the final output resource we will need eTransferSrc as it will be blitted to the
    // swapchain images.
    resources[mOutputResource] |= vk::ImageUsageFlagBits::eTransferSrc;

    // Now we will create a PhysicalResource for each LogicalResource.
    for (auto resource : resources) {
      vk::ImageAspectFlags aspect;

      if (Utils::isDepthOnlyFormat(resource.first->mFormat)) {
        aspect |= vk::ImageAspectFlagBits::eDepth;
      } else if (Utils::isDepthStencilFormat(resource.first->mFormat)) {
        aspect |= vk::ImageAspectFlagBits::eDepth;
        aspect |= vk::ImageAspectFlagBits::eStencil;
      } else {
        aspect |= vk::ImageAspectFlagBits::eColor;
      }

      vk::ImageCreateInfo imageInfo;
      imageInfo.imageType     = vk::ImageType::e2D;
      imageInfo.format        = resource.first->mFormat;
      imageInfo.extent.width  = resource.first->mExtent.x;
      imageInfo.extent.height = resource.first->mExtent.y;
      imageInfo.extent.depth  = 1;
      imageInfo.mipLevels     = 1;
      imageInfo.arrayLayers   = 1;
      imageInfo.samples       = resource.first->mSamples;
      imageInfo.tiling        = vk::ImageTiling::eOptimal;
      imageInfo.usage         = resource.second;
      imageInfo.sharingMode   = vk::SharingMode::eExclusive;
      imageInfo.initialLayout = vk::ImageLayout::eUndefined;

      perFrame.mPhysicalResources[resource.first] = {
          mDevice->createBackedImage("Resource \"" + resource.first->mName + "\" of " + getName(),
              imageInfo, vk::ImageViewType::e2D, aspect, vk::MemoryPropertyFlagBits::eDeviceLocal,
              vk::ImageLayout::eUndefined)};
    }

    // Create FrameBuffers for each RenderPass -----------------------------------------------------

    // Create a RenderPass for each PhysicalPass ---------------------------------------------------

    perFrame.mDirty = false;
    Core::Logger::debug() << "Frame graph construction done." << std::endl;
  }

  // -------------------------------------------------------------------------------------------- //
  // ------------------------------------ recording phase --------------------------------------- //
  // -------------------------------------------------------------------------------------------- //

  // Now we can finally start recording our command buffer.
  perFrame.mPrimaryCommandBuffer->reset();
  perFrame.mPrimaryCommandBuffer->begin();

  // perFrame.mPrimaryCommandBuffer->beginRenderPass(res.mRenderPass);

  mThreadPool.setThreadCount(threadCount);

  for (auto& physicalPass : perFrame.mPhysicalPasses) {
    for (auto& pass : physicalPass.mSubPasses) {
      if (pass->mProcessCallback) {
        mThreadPool.enqueue(
            [pass, perFrame]() { pass->mProcessCallback(perFrame.mPrimaryCommandBuffer); });
      }
    }
  }

  mThreadPool.waitIdle();

  // perFrame.mPrimaryCommandBuffer->endRenderPass();

  perFrame.mPrimaryCommandBuffer->end();

  perFrame.mPrimaryCommandBuffer->submit({}, {}, {perFrame.mRenderFinishedSemaphore});

  mOutputWindow->present(perFrame.mPhysicalResources[mOutputResource].mImage,
      perFrame.mRenderFinishedSemaphore, perFrame.mFrameFinishedFence);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameGraph::isDirty() const {
  if (mDirty) {
    return true;
  }

  for (auto const& resource : mLogicalResources) {
    if (resource.mDirty) {
      return true;
    }
  }

  for (auto const& pass : mLogicalPasses) {
    if (pass.mDirty) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::clearDirty() {
  mDirty = false;

  for (auto& resource : mLogicalResources) {
    resource.mDirty = false;
  }

  for (auto& pass : mLogicalPasses) {
    pass.mDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::validate() const {

  Core::Logger::debug() << "Validating frame graph..." << std::endl;

  // Check whether each resource of each pass was actually created by this frame graph.
  for (auto const& pass : mLogicalPasses) {
    for (auto const& passResource : pass.mLogicalResources) {
      bool ours = false;
      for (auto const& graphResource : mLogicalResources) {
        if (&graphResource == passResource.first) {
          ours = true;
          break;
        }
      }
      if (!ours) {
        throw std::runtime_error("Resource \"" + passResource.first->mName + "\" of pass \"" +
                                 pass.mName +
                                 "\" does not belong to this frame graph. Did you accidentally "
                                 "create a copy of the reference?");
      }
    }
  }

  // Check whether we have a valid output window, pass and resource.
  if (!mOutputWindow) {
    throw std::runtime_error("There is no output window set!");
  }

  if (!mOutputPass) {
    throw std::runtime_error("There is no output pass set!");
  }

  if (!mOutputResource) {
    throw std::runtime_error("There is no output resource set!");
  }

  // Check whether the output pass actually belongs to this graph.
  bool isOurOutputPass = false;
  for (auto const& pass : mLogicalPasses) {
    if (mOutputPass == &pass) {
      isOurOutputPass = true;
      break;
    }
  }

  if (!isOurOutputPass) {
    throw std::runtime_error("The output pass \"" + mOutputPass->mName +
                             "\" does not belong to this frame graph. Did you accidentally "
                             "create a copy of the reference?");
  }

  // Check whether the output resource actually belongs to the output pass.
  bool isOurOutputResource = false;
  for (auto const& resource : mOutputPass->mLogicalResources) {
    if (mOutputResource == resource.first) {
      isOurOutputResource = true;
      break;
    }
  }
  if (!isOurOutputResource) {
    throw std::runtime_error("Output resource \"" + mOutputResource->mName +
                             "\" does not belong to output pass \"" + mOutputPass->mName +
                             "\".. Did you accidentally create a copy of the reference?");
  }

  // Check whether the resolutions of all attachments of each pass are the same
  glm::ivec2 windowExtent = mOutputWindow->pExtent.get();
  for (auto const& pass : mLogicalPasses) {
    glm::ivec2 passExtent = glm::ivec2(-1);
    for (auto const& resource : pass.mLogicalResources) {
      glm::ivec2 resourceExtent = resource.first->getAbsoluteExtent(windowExtent);
      if (passExtent == glm::ivec2(-1)) {
        passExtent = resourceExtent;
      } else if (passExtent != resourceExtent) {
        throw std::runtime_error(
            "Attachments of pass \"" + pass.mName + "\" do not have the same size!");
      }
    }
  }

  Core::Logger::debug() << "  all good." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
