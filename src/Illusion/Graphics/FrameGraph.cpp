////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FrameGraph.hpp"

#include "../Core/Logger.hpp"
#include "../Core/Utils.hpp"
#include "BackedImage.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Utils.hpp"
#include "Window.hpp"

#include <queue>
#include <unordered_set>

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

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setExtent(glm::vec2 const& extent) {
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
  assignResource(resource, access, {});
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::LogicalPass::assignResource(
    LogicalResource const& resource, vk::ClearValue const& clear) {
  assignResource(resource, ResourceAccess::eWriteOnly, clear);
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
  mDirty           = true;
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
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Resource has already been added to this pass!");
  }

  // We cannot add multiple depth attachments.
  if (resource.isDepthResource() && getDepthAttachment() != nullptr) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Pass already has a depth attachment!");
  }

  mLogicalResources[&resource] = access;

  if (clear) {
    mClearValues[&resource] = *clear;
  }

  mDirty = true;
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

void FrameGraph::process(ProcessingFlags flags) {

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
  // * Create a secondary CommandBuffer for each LogicalPass
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
      PhysicalPass physicalPass({{{logicalPass}}});
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
            if (r.second == ResourceAccess::eWriteOnly) {
              throw std::runtime_error("Frame graph construction failed: Write-only output \"" +
                                       r.first->mName + "\" of pass \"" + logicalPass->mName +
                                       "\" is used by the preceding pass \"" + previousPass->mName +
                                       "\"!");
            } else if (previousUse->second == ResourceAccess::eReadOnly ||
                       previousUse->second == ResourceAccess::eLoad) {
              Core::Logger::debug()
                  << "      is read-only in pass \"" + previousPass->mName + "\"." << std::endl;
            } else {
              // In order to make sure that there are no duplicates in our queue, we first remove
              // all entries referencing the same pass.
              passQueue.remove(&(*previousPass));
              passQueue.push_back(&(*previousPass));
              physicalPass.mSubpasses[0].mDependencies.insert(&(*previousPass));

              Core::Logger::debug()
                  << "      is written by pass \"" + previousPass->mName + "\"." << std::endl;
            }
          } else if (physicalPass.mSubpasses[0].mDependencies.size() == 0) {
            if (r.second == ResourceAccess::eWriteOnly) {
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
      Core::Logger::debug() << "    Pass " << p.mSubpasses[0].mLogicalPass->mName << std::endl;
    }

    // Merge adjacent PhysicalPasses which can be executed as sub passes ---------------------------

    // Now we can merge adjacent PhysicalPasses which have the same extent and share the same depth
    // attachment. To do this, we traverse the list of PhysicalPasses front-to-back and for each
    // pass we search for candidates sharing extent, depth attachment and dependencies.
    auto current = perFrame.mPhysicalPasses.begin();
    while (current != perFrame.mPhysicalPasses.end()) {

      // Keep a reference to the current depth attachment. This may be nullptr, in this case we
      // accept any depth attachment of the candidates.
      auto currentDepthAttachment = current->mSubpasses[0].mLogicalPass->getDepthAttachment();

      // Now look for merge candidates. We start with the next pass.
      auto candidate = current;
      ++candidate;

      while (candidate != perFrame.mPhysicalPasses.end()) {
        // For each candidate, the depth attachment must be the same as the current passes depth
        // attachment. Or either may be nullptr.
        auto candidateDepthAttachment = candidate->mSubpasses[0].mLogicalPass->getDepthAttachment();
        bool sameDepthAttachment      = currentDepthAttachment == nullptr ||
                                   candidateDepthAttachment == nullptr ||
                                   candidateDepthAttachment == currentDepthAttachment;

        // They must share the same extent.
        bool extentMatches = candidate->mExtent == current->mExtent;

        // And all dependencies of the candidate must be satisfied. That means all dependencies of
        // the candidate must either be dependencies of the first sub-pass of the current physical
        // pass or they must be a sub-pass of the current physical pass.
        bool dependenciesSatisfied = true;

        for (auto const& d : candidate->mSubpasses[0].mDependencies) {
          bool isDependencyOfFirst = Core::Utils::contains(current->mSubpasses[0].mDependencies, d);

          bool isSubpassOfCurrent = false;
          for (auto const& subpass : current->mSubpasses) {
            if (subpass.mLogicalPass == d) {
              isSubpassOfCurrent = true;
              break;
            }
          }

          if (!isDependencyOfFirst && !isSubpassOfCurrent) {
            dependenciesSatisfied = false;
            break;
          }
        }

        // If all conditions are fulfilled, we can make the candidate a sub pass of the current
        // PhysicalPass. If the current depth attachment has been nullptr, we can now use the depth
        // attachment of the candidate for further candidate checks.
        if (extentMatches && sameDepthAttachment && dependenciesSatisfied) {
          current->mSubpasses.push_back(candidate->mSubpasses[0]);

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

    // Assign a name to each PhysicalPass to allow for descriptive error / warning / debug output.
    uint32_t counter = 0;
    for (auto& pass : perFrame.mPhysicalPasses) {
      std::vector<std::string> subpassNames;
      for (auto const& subpass : pass.mSubpasses) {
        subpassNames.push_back("\"" + subpass.mLogicalPass->mName + "\"");
      }

      pass.mName = "RenderPass " + std::to_string(counter++) + " (" +
                   Core::Utils::joinStrings(subpassNames, ", ", " and ") + ")";
    }

    // Print some debugging information.
    Core::Logger::debug() << "  Physical pass execution order will be" << std::endl;
    for (auto const& pass : perFrame.mPhysicalPasses) {
      Core::Logger::debug() << "    " << pass.mName << std::endl;
    }

    // Identify resource usage per PhysicalPass ----------------------------------------------------

    // For each sub-pass we will collect information on how the individual resources are used.
    // This information is stored in the mResourceUsage member of the PhysicalPass::Subpass.
    for (auto& pass : perFrame.mPhysicalPasses) {
      for (auto& subpass : pass.mSubpasses) {
        for (auto const& r : subpass.mLogicalPass->mLogicalResources) {
          switch (r.second) {
          case ResourceAccess::eReadOnly:
            subpass.mResourceUsage[r.first] |= vk::ImageUsageFlagBits::eInputAttachment;
            break;
          case ResourceAccess::eLoad:
          case ResourceAccess::eWriteOnly:
          case ResourceAccess::eLoadWrite:
            if (r.first->isColorResource()) {
              subpass.mResourceUsage[r.first] |= vk::ImageUsageFlagBits::eColorAttachment;
            } else if (r.first->isDepthResource()) {
              subpass.mResourceUsage[r.first] |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            }
            break;
          case ResourceAccess::eReadWrite:
          case ResourceAccess::eLoadReadWrite:
            subpass.mResourceUsage[r.first] |= vk::ImageUsageFlagBits::eInputAttachment;
            if (r.first->isColorResource()) {
              subpass.mResourceUsage[r.first] |= vk::ImageUsageFlagBits::eColorAttachment;
            } else if (r.first->isDepthResource()) {
              subpass.mResourceUsage[r.first] |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            }
            break;
          }
        }
      }
    }

    // Similarly, we collect the clear-values for each resource. This information is required when
    // we begin the RenderPass later.
    for (auto& pass : perFrame.mPhysicalPasses) {
      for (auto const& subpass : pass.mSubpasses) {
        pass.mClearValues.insert(
            subpass.mLogicalPass->mClearValues.begin(), subpass.mLogicalPass->mClearValues.end());
      }
    }

    // Create a BackedImage for each PhysicalResource ----------------------------------------------

    // Now we will create actual PhysicalResources. We will create everything from scratch here.
    // This can definitely be optimized, but for now frequent graph changes are not planned anyways.
    perFrame.mPhysicalResources.clear();

    // First we will create a map of resources which are actually used by the passes we collected.
    // For each resource we will accumulate the vk::ImageUsageFlagBits.
    std::unordered_map<LogicalResource const*, vk::ImageUsageFlags> resourceUsage;
    for (auto& pass : perFrame.mPhysicalPasses) {
      for (auto& subpass : pass.mSubpasses) {
        for (auto const& resource : subpass.mResourceUsage) {
          resourceUsage[resource.first] |= resource.second;
        }
      }
    }

    // For the final output resource we will need eTransferSrc as it will be blitted to the
    // swapchain images.
    resourceUsage[mOutputResource] |= vk::ImageUsageFlagBits::eTransferSrc;

    // Now we will create a BackedImage for each PhysicalResource.
    for (auto resource : resourceUsage) {
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

    // Create a RenderPass for each PhysicalPass ---------------------------------------------------

    // For each PhysicalPass we will create a RenderPass, attach the PhysicalResources we just
    // created and setup the sub-passes.
    for (auto& pass : perFrame.mPhysicalPasses) {

      // First we have to create an "empty" RenderPass.
      pass.mRenderPass = RenderPass::create(pass.mName, mDevice);

      // Then we have to collect all PhysicalResources which are required for this pass in the
      // mAttachments vector of each PhysicalPass. We could use a std::unordered_set here to remove
      // duplicates, but we would like to have a predictable order of attachments so we rather go
      // for a std::vector and check for duplicates ourselves.
      // At the same time we setup the SubpassInfo structures for the RenderPass. These contain
      // information on the dependencies between sub-passes and their resource usage.
      std::vector<RenderPass::SubpassInfo> subpassInfos;

      for (auto const& subpass : pass.mSubpasses) {
        RenderPass::SubpassInfo subpassInfo;

        for (auto const& resource : subpass.mLogicalPass->mLogicalResources) {

          // Add the resource to the resources vector. The content of the resources vector will be
          // the order of attachments of our framebuffer in the end.
          auto resourceIt =
              std::find(pass.mAttachments.begin(), pass.mAttachments.end(), resource.first);
          if (resourceIt == pass.mAttachments.end()) {
            pass.mAttachments.push_back(resource.first);
            resourceIt = pass.mAttachments.end() - 1;
          }

          // We calculate the index of the current resource and, depending on the usage, add this
          // index either as input attachment, as output attachment or as both to our SubpassInfo
          // structure.
          uint32_t resourceIdx = std::distance(pass.mAttachments.begin(), resourceIt);

          if (resource.second == ResourceAccess::eReadOnly ||
              resource.second == ResourceAccess::eReadWrite ||
              resource.second == ResourceAccess::eLoadReadWrite) {
            subpassInfo.mInputAttachments.push_back(resourceIdx);
          }

          if (resource.second == ResourceAccess::eWriteOnly ||
              resource.second == ResourceAccess::eReadWrite ||
              resource.second == ResourceAccess::eLoad ||
              resource.second == ResourceAccess::eLoadWrite ||
              resource.second == ResourceAccess::eLoadReadWrite) {
            subpassInfo.mOutputAttachments.push_back(resourceIdx);
          }
        }

        // Now we have to setup the sub-pass dependencies for our RenderPass. That means for each
        // dependency of the current sub-pass we will check whether this is actually part of the
        // same RenderPass. If so, that dependency is a sub-pass dependency.
        for (auto dependency : subpass.mDependencies) {
          for (size_t i(0); i < pass.mSubpasses.size(); ++i) {
            if (dependency == pass.mSubpasses[i].mLogicalPass) {
              subpassInfo.mPreSubpasses.push_back(i);
            }
          }
        }

        subpassInfos.push_back(subpassInfo);
      }

      // Then we can add the collected resources to our RenderPass as attachments.
      Core::Logger::debug() << "  Adding attachments to " << pass.mRenderPass->getName()
                            << std::endl;

      for (auto passResource : pass.mAttachments) {
        auto physicalResource = &perFrame.mPhysicalResources[passResource];
        Core::Logger::debug() << "    " << physicalResource->mImage->mName << std::endl;
        pass.mRenderPass->addAttachment(physicalResource->mImage);
      }

      // And set the sub-pass info structures.
      pass.mRenderPass->setSubpasses(subpassInfos);
    }

    // Create a secondary CommandBuffer for each LogicalPass ---------------------------------------

    // Since LogicalPasses can be recorded in parallel, we need separate secondary CommandBuffers
    // for each one.
    for (auto& pass : perFrame.mPhysicalPasses) {
      for (auto& subpass : pass.mSubpasses) {
        subpass.mSecondaryCommandBuffer = CommandBuffer::create(subpass.mLogicalPass->mName,
            mDevice, QueueType::eGeneric, vk::CommandBufferLevel::eSecondary);
      }
    }

    // We are done! A new frame graph has been constructed.
    perFrame.mDirty = false;
    Core::Logger::debug() << "Frame graph construction done." << std::endl;
  }

  // -------------------------------------------------------------------------------------------- //
  // ------------------------------------ recording phase --------------------------------------- //
  // -------------------------------------------------------------------------------------------- //

  // Now we can finally start recording our command buffer.
  perFrame.mPrimaryCommandBuffer->reset();
  perFrame.mPrimaryCommandBuffer->begin();

  if (flags.has(ProcessingFlagBits::eParallelRenderPassRecording)) {
    throw std::runtime_error("Parallel RenderPass recording is not implemented yet!");
  }

  if (flags.has(ProcessingFlagBits::eParallelSubpassRecording)) {
    mThreadPool.setThreadCount(0);
  } else {
    mThreadPool.setThreadCount(1);
  }

  for (auto const& pass : perFrame.mPhysicalPasses) {

    std::vector<vk::ClearValue> clearValues;
    for (auto attachment : pass.mAttachments) {
      if (pass.mClearValues.find(attachment) != pass.mClearValues.end()) {
        clearValues.push_back(pass.mClearValues.at(attachment));
      } else {
        clearValues.push_back({});
      }
    }

    perFrame.mPrimaryCommandBuffer->beginRenderPass(
        pass.mRenderPass, clearValues, vk::SubpassContents::eSecondaryCommandBuffers);

    uint32_t subpassCounter = 0;
    for (auto const& subpass : pass.mSubpasses) {
      mThreadPool.enqueue([pass, subpass, subpassCounter]() {
        vk::CommandBufferInheritanceInfo info;
        info.subpass     = subpassCounter;
        info.renderPass  = *pass.mRenderPass->getHandle();
        info.framebuffer = *pass.mRenderPass->getFramebuffer();
        subpass.mSecondaryCommandBuffer->begin(info);
        subpass.mLogicalPass->mProcessCallback(subpass.mSecondaryCommandBuffer);
        subpass.mSecondaryCommandBuffer->end();
      });
      ++subpassCounter;
    }

    mThreadPool.waitIdle();

    subpassCounter = 0;
    for (auto const& subpass : pass.mSubpasses) {
      perFrame.mPrimaryCommandBuffer->execute(subpass.mSecondaryCommandBuffer);

      if (++subpassCounter < pass.mSubpasses.size()) {
        perFrame.mPrimaryCommandBuffer->nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);
      }
    }

    perFrame.mPrimaryCommandBuffer->endRenderPass();
  }

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

  // Check whether each pass actually has a process-callback.
  for (auto const& pass : mLogicalPasses) {
    if (!pass.mProcessCallback) {
      throw std::runtime_error("Pass \"" + pass.mName + "\" has no process-callback set!");
    }
  }

  Core::Logger::debug() << "  all good." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
