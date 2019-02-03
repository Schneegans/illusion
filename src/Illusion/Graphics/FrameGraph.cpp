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

FrameGraph::Resource& FrameGraph::Resource::setName(std::string const& name) {
  mName = name;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Resource& FrameGraph::Resource::setFormat(vk::Format format) {
  mFormat = format;
  mDirty  = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Resource& FrameGraph::Resource::setSizing(Sizing sizing) {
  mSizing = sizing;
  mDirty  = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Resource& FrameGraph::Resource::setExtent(glm::vec2 const& extent) {
  mExtent = extent;
  mDirty  = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Resource& FrameGraph::Resource::setSamples(vk::SampleCountFlagBits const& samples) {
  mSamples = samples;
  mDirty   = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 FrameGraph::Resource::getAbsoluteExtent(glm::uvec2 const& windowExtent) const {
  return (mSizing == Sizing::eAbsolute) ? glm::uvec2(mExtent)
                                        : glm::uvec2(mExtent * glm::vec2(windowExtent));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Pass& FrameGraph::Pass::addColorAttachment(
    Resource const& resource, AccessFlags access, std::optional<vk::ClearColorValue> clear) {

  // We cannot add the same resource twice.
  if (Core::Utils::contains(mResourceAccess, &resource)) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Resource has already been added to this pass!");
  }

  // Access for cleared attachments must be write-only.
  if (clear && !access.containsOnly(AccessFlagBits::eWrite)) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Resource which are cleared must be write-only!");
  }

  vk::ImageUsageFlags usage;

  if (access.contains(AccessFlagBits::eRead)) {
    usage |= vk::ImageUsageFlagBits::eInputAttachment;
  }

  if (access.contains(AccessFlagBits::eWrite) || access.contains(AccessFlagBits::eLoad)) {
    usage |= vk::ImageUsageFlagBits::eColorAttachment;
  }

  mResourceAccess[&resource] = access;
  mResourceUsage[&resource]  = usage;

  if (clear) {
    mResourceClear[&resource] = *clear;
  }

  mDirty = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Pass& FrameGraph::Pass::addDepthAttachment(
    Resource const& resource, AccessFlags access, std::optional<vk::ClearDepthStencilValue> clear) {

  // Depth attachments cannot be read.
  if (access.contains(AccessFlagBits::eRead)) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Depth attachments can not be read!");
  }

  // Access for cleared attachments must be eWriteOnly.
  if (clear && !access.containsOnly(AccessFlagBits::eWrite)) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Resource which are cleared must be eWriteOnly!");
  }

  // We cannot add the same resource twice.
  if (Core::Utils::contains(mResourceAccess, &resource)) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Resource has already been added to this pass!");
  }

  // We cannot add multiple depth attachments.
  for (auto const& usage : mResourceUsage) {
    if (VkFlags(usage.second & vk::ImageUsageFlagBits::eDepthStencilAttachment) > 0) {
      throw std::runtime_error("Failed to add resource \"" + resource.mName +
                               "\" to frame graph pass \"" + mName +
                               "\": Pass already has a depth attachment!");
    }
  }

  mResourceAccess[&resource] = access;
  mResourceUsage[&resource]  = vk::ImageUsageFlagBits::eDepthStencilAttachment;

  if (clear) {
    mResourceClear[&resource] = *clear;
  }

  mDirty = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Pass& FrameGraph::Pass::setName(std::string const& name) {
  mName = name;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Pass& FrameGraph::Pass::setProcessCallback(
    std::function<void(CommandBufferPtr)> const& callback) {
  mProcessCallback = callback;
  mDirty           = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::FrameGraph(
    std::string const& name, DevicePtr const& device, FrameResourceIndexPtr const& index)
    : Core::NamedObject(name)
    , mDevice(device)
    , mPerFrame(index, [device, this](uint32_t i) {
      PerFrame    perFrame;
      std::string prefix                = std::to_string(i) + " of " + getName();
      perFrame.mPrimaryCommandBuffer    = CommandBuffer::create("CommandBuffer " + prefix, device);
      perFrame.mRenderFinishedSemaphore = device->createSemaphore("RenderFinished " + prefix);
      perFrame.mFrameFinishedFence      = device->createFence("FrameFinished " + prefix);
      return perFrame;
    }) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Resource& FrameGraph::createResource() {
  mDirty = true;
  mResources.push_back({});
  return mResources.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::Pass& FrameGraph::createPass() {
  mDirty = true;
  mPasses.push_back({});
  return mPasses.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::setOutput(WindowPtr const& window, Pass const& pass, Resource const& resource) {
  mOutputWindow     = window;
  mOutputPass       = &pass;
  mOutputAttachment = &resource;
  mDirty            = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::process(ProcessingFlags flags) {

  // -------------------------------------------------------------------------------------------- //
  // ---------------------------------- graph validation phase ---------------------------------- //
  // -------------------------------------------------------------------------------------------- //

  // The FrameGraph is dirty when a Pass or Resource was added, when a Resource
  // was added to one of its Passes or when one of its Resources was reconfigured.
  if (isDirty()) {

    // First make some sanity checks. After this call we can be sure that all references to
    // Resources in the Passes are valid.
    try {
      validate();
    } catch (std::runtime_error e) {
      throw std::runtime_error("Frame graph validation failed: " + std::string(e.what()));
    }

    // Then make sure that we re-create all per-frame render passes and physical resources.
    for (auto& perFrame : mPerFrame) {
      perFrame.mDirty = true;
    }

    // Reset dirty flags of the FrameGraph, the Passes and the Resources.
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

  // If perFrame.mDirty is set, the render passes and physical resources need to be updated. This
  // could definitely be optimized with more fine-grained dirty flags, but as this should not happen
  // on a frame-to-frame basis, it seems to be ok to recreate everything from scratch here. To give
  // an overview of the code below, here is a rough outline:
  // * Create a list of RenderPassInfos, one for each reachable Pass.
  //   * Find the final output pass.
  //   * Recursively add required input passes to the list.
  //   * Reverse the list
  // * Merge adjacent RenderPassInfos which can be executed as subpasses
  // * Create a BackedImage for each Resource
  // * Create a RenderPass for each RenderPassInfo
  // * Create a secondary CommandBuffer for each RenderPass
  if (perFrame.mDirty) {
    Core::Logger::debug() << "Constructing frame graph ..." << std::endl;

    // Compute logical pass execution order --------------------------------------------------------

    perFrame.mRenderPasses.clear();

    // First we will create a list of RenderPassInfos with a valid execution order. This list may
    // contain less passes than this FrameGraph has Passes, as some passes may not be connected to
    // our final pass (pass culling). We will collect the passes bottom-up; that means we start with
    // the final output pass and then collect all passes which provide input for this pass. Then we
    // search for the passes which provide input for those passes... and so on.
    // To do this, we will create a queue (actually we use a std::list as we have to remove
    // duplicates) of inputs which are required for the processing of the passes inserted into our
    // mRenderPasses. We will start with the final output pass.
    std::list<Pass const*> passQueue;
    passQueue.push_back(mOutputPass);

    Core::Logger::debug() << "  Resolving pass dependencies ..." << std::endl;

    while (!passQueue.empty()) {
      // Pop the current pass from the queue.
      auto pass = passQueue.front();
      passQueue.pop_front();

      // Skip passes without any resources.
      if (pass->mResourceAccess.size() == 0) {
        Core::Logger::debug() << "    Skipping pass \"" + pass->mName +
                                     "\" because it has no resources assigned."
                              << std::endl;
        continue;
      }

      // And create a RenderPassInfo for it. We store the extent of the pass for easier later
      // access. Due to the previous graph validation we are sure that all resources have the same
      // resolution.
      RenderPassInfo renderPass;
      renderPass.mSubpasses.push_back({pass});
      for (auto const& resource : pass->mResourceAccess) {
        renderPass.mExtent = resource.first->getAbsoluteExtent(mOutputWindow->pExtent.get());
        break;
      }

      Core::Logger::debug() << "    Resolving dependencies of pass \"" + pass->mName + "\"..."
                            << std::endl;

      // We start searching for preceding passes providing required resources at our current pass.
      auto currentPass = mPasses.rbegin();
      while (&(*currentPass) != pass) {
        ++currentPass;
      }

      // Now we have to find the passes which are in front of the current pass in the mPasses list
      // of the FrameGraph and write to to the resources of the current pass.
      for (auto r : pass->mResourceAccess) {
        Core::Logger::debug() << "      resource \"" + r.first->mName + "\"" << std::endl;

        // Step backwards through all Passes, collecting all passes writing to this resource.
        auto previousPass = currentPass;
        auto previousUse  = previousPass->mResourceAccess.find(r.first);

        while (previousPass != mPasses.rend()) {
          do {
            ++previousPass;
          } while (previousPass != mPasses.rend() &&
                   (previousUse = previousPass->mResourceAccess.find(r.first)) ==
                       previousPass->mResourceAccess.end());

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
          if (previousPass != mPasses.rend()) {
            if (r.second.containsOnly(AccessFlagBits::eWrite)) {
              throw std::runtime_error("Frame graph construction failed: Write-only output \"" +
                                       r.first->mName + "\" of pass \"" + pass->mName +
                                       "\" is used by the preceding pass \"" + previousPass->mName +
                                       "\"!");
            } else if (!previousUse->second.contains(AccessFlagBits::eWrite)) {
              Core::Logger::debug()
                  << "        is read-only in pass \"" + previousPass->mName + "\"." << std::endl;
            } else {
              // In order to make sure that there are no duplicates in our queue, we first remove
              // all entries referencing the same pass.
              passQueue.remove(&(*previousPass));
              passQueue.push_back(&(*previousPass));
              renderPass.mSubpasses[0].mDependencies.insert(&(*previousPass));

              Core::Logger::debug()
                  << "        is written by pass \"" + previousPass->mName + "\"." << std::endl;
            }
          } else if (renderPass.mSubpasses[0].mDependencies.size() == 0) {
            if (r.second.containsOnly(AccessFlagBits::eWrite)) {
              Core::Logger::debug() << "        is created by this pass." << std::endl;
            } else {
              throw std::runtime_error("Frame graph construction failed: Input \"" +
                                       r.first->mName + "\" of pass \"" + pass->mName +
                                       "\" is not write-only but no previous pass writes to it!");
            }
          }
        }
      }

      // Finally we can add the renderPass to the list.
      perFrame.mRenderPasses.push_back(renderPass);
    }

    Core::Logger::debug() << "  Pass dependencies successfully resolved." << std::endl;

    // Now we have to reverse our list of passes as we collected it bottom-up.
    perFrame.mRenderPasses.reverse();

    // Print some debugging information.
    Core::Logger::debug() << "  Logical pass execution order will be:" << std::endl;
    uint32_t counter = 0;
    for (auto const& p : perFrame.mRenderPasses) {
      Core::Logger::debug() << "    Pass " << counter++ << " (\"" << p.mSubpasses[0].mPass->mName
                            << "\")" << std::endl;
    }

    // Merge adjacent RenderPassInfos which can be executed as subpasses ---------------------------

    // Now we can merge adjacent RenderPassInfos which have the same extent. To do this, we traverse
    // the list of RenderPassInfos front-to-back and for each pass we search for candidates sharing
    // extent, depth attachment and dependencies.
    auto current = perFrame.mRenderPasses.begin();
    while (current != perFrame.mRenderPasses.end()) {

      // Now look for merge candidates. We start with the next pass.
      auto candidate = current;
      ++candidate;

      while (candidate != perFrame.mRenderPasses.end()) {

        // They must share the same extent.
        bool extentMatches = candidate->mExtent == current->mExtent;

        // And all dependencies of the candidate must be satisfied. That means all dependencies of
        // the candidate must either be dependencies of the first subpass of the current render
        // pass or they must be a subpass of the current render pass.
        bool dependenciesSatisfied = true;

        for (auto const& d : candidate->mSubpasses[0].mDependencies) {
          bool isDependencyOfFirst = Core::Utils::contains(current->mSubpasses[0].mDependencies, d);

          bool isSubpassOfCurrent = false;
          for (auto const& subpass : current->mSubpasses) {
            if (subpass.mPass == d) {
              isSubpassOfCurrent = true;
              break;
            }
          }

          if (!isDependencyOfFirst && !isSubpassOfCurrent) {
            dependenciesSatisfied = false;
            break;
          }
        }

        // If all conditions are fulfilled, we can make the candidate a subpass of the current
        // RenderPassInfo.
        if (extentMatches && dependenciesSatisfied) {
          current->mSubpasses.push_back(candidate->mSubpasses[0]);

          // Erase the candidate from the list of RenderPassInfos and look for more merge candidates
          // in the next iteration.
          candidate = perFrame.mRenderPasses.erase(candidate);
        } else {
          // Look for more merge candidates in the next iteration.
          ++candidate;
        }
      }

      // Look for potential merge candidates of the next RenderPassInfo.
      ++current;
    }

    // Assign a name to each RenderPassInfo to allow for descriptive error / warning / debug output.
    counter = 0;
    for (auto& pass : perFrame.mRenderPasses) {
      std::vector<std::string> subpassNames;
      for (auto const& subpass : pass.mSubpasses) {
        subpassNames.push_back("\"" + subpass.mPass->mName + "\"");
      }

      pass.mName = "RenderPass " + std::to_string(counter++) + " (" +
                   Core::Utils::joinStrings(subpassNames, ", ", " and ") + ")";
    }

    // Print some debugging information.
    Core::Logger::debug() << "  Physical execution order will be:" << std::endl;
    for (auto const& pass : perFrame.mRenderPasses) {
      Core::Logger::debug() << "    " << pass.mName << std::endl;
    }

    // Identify resource usage per RenderPassInfo --------------------------------------------------

    // For each subpass we will collect information on how the individual resources are used on what
    // level. This information is stored in the mResourceUsage and mResourceAccess members of the
    // SubpassInfo and the RenderPassInfo. Similarly, we collect the clear-values for each resource.
    // This information is required when we begin the RenderPass later.
    for (auto& pass : perFrame.mRenderPasses) {
      for (auto& subpass : pass.mSubpasses) {
        for (auto const& r : subpass.mPass->mResourceAccess) {
          pass.mResourceAccess[r.first] |= r.second;
        }
        for (auto const& r : subpass.mPass->mResourceUsage) {
          pass.mResourceUsage[r.first] |= r.second;
        }
        for (auto const& resource : subpass.mPass->mResourceClear) {
          pass.mResourceClear[resource.first] = resource.second;
        }
      }
    }

    // We will also create a map of resources which are actually used by all the passes we
    // collected. For each resource we will accumulate the vk::ImageUsageFlagBits.
    std::unordered_map<Resource const*, vk::ImageUsageFlags> overallResourceUsage;
    for (auto const& pass : perFrame.mRenderPasses) {
      for (auto const& resource : pass.mResourceUsage) {
        overallResourceUsage[resource.first] |= resource.second;
      }
    }

    // For the final output resource we will need eTransferSrc as it will be blitted to the
    // swapchain images.
    overallResourceUsage[mOutputAttachment] |= vk::ImageUsageFlagBits::eTransferSrc;

    // Create a RenderPass for each RenderPassInfo and BackedImages for each Resource --------------

    // Now we will create actual physical resources. We will create everything from scratch here.
    // This can definitely be optimized, but for now frequent graph changes are not planned anyways.
    perFrame.mAllAttachments.clear();

    // For each RenderPassInfo we will create a RenderPass, attach the physical resources we just
    // created and setup the subpasses.
    for (auto passIt = perFrame.mRenderPasses.begin(); passIt != perFrame.mRenderPasses.end();
         ++passIt) {

      // First we have to create an "empty" RenderPass.
      passIt->mRenderPass = RenderPass::create(passIt->mName, mDevice);

      // Then we have to collect all physical resources which are required for this pass in the
      // mAttachments vector of each RenderPassInfo. We could use a std::unordered_set here to
      // remove duplicates, but we would like to have a predictable order of attachments so we
      // rather go for a std::vector and check for duplicates ourselves.
      // At the same time we setup the Subpass structures for the RenderPass. These contain
      // information on the dependencies between subpasses and their resource usage.
      std::vector<RenderPass::Subpass> subpasses;

      for (auto const& subpassInfo : passIt->mSubpasses) {
        RenderPass::Subpass subpass;

        for (auto const& resource : subpassInfo.mPass->mResourceAccess) {

          // Add the resource to the resources vector. The content of the resources vector will be
          // the order of attachments of our framebuffer in the end.
          auto resourceIt =
              std::find(passIt->mAttachments.begin(), passIt->mAttachments.end(), resource.first);
          if (resourceIt == passIt->mAttachments.end()) {
            passIt->mAttachments.push_back(resource.first);
            resourceIt = passIt->mAttachments.end() - 1;
          }

          // We calculate the index of the current resource and, depending on the usage, add this
          // index either as input attachment, as output attachment or as both to our Subpass
          // structure.
          uint32_t resourceIdx = std::distance(passIt->mAttachments.begin(), resourceIt);

          if (resource.second.contains(AccessFlagBits::eRead)) {
            subpass.mInputAttachments.push_back(resourceIdx);
          }

          if (resource.second.contains(AccessFlagBits::eWrite) ||
              resource.second.contains(AccessFlagBits::eLoad)) {
            if (VkFlags(subpassInfo.mPass->mResourceUsage.at(resource.first) &
                        vk::ImageUsageFlagBits::eDepthStencilAttachment) > 0) {
              subpass.mDepthStencilAttachment = resourceIdx;
            } else {
              subpass.mColorAttachments.push_back(resourceIdx);
            }
          }
        }

        // Now we have to setup the subpass dependencies for our RenderPass. That means for each
        // dependency of the current subpass we will check whether this is actually part of the
        // same RenderPass. If so, that dependency is a subpass dependency.
        for (auto dependency : subpassInfo.mDependencies) {
          for (size_t i(0); i < passIt->mSubpasses.size(); ++i) {
            if (dependency == passIt->mSubpasses[i].mPass) {
              subpass.mPreSubpasses.push_back(i);
            }
          }
        }

        subpasses.push_back(subpass);
      }

      // Then we can add the collected resources to our RenderPass as attachments.
      Core::Logger::debug() << "  Adding attachments to " << passIt->mRenderPass->getName()
                            << std::endl;

      for (auto attachment : passIt->mAttachments) {

        RenderPass::Attachment attachmentInfo;
        attachmentInfo.mInitialLayout = vk::ImageLayout::eUndefined;
        attachmentInfo.mFinalLayout   = vk::ImageLayout::eUndefined;
        attachmentInfo.mLoadOp        = vk::AttachmentLoadOp::eDontCare;
        attachmentInfo.mStoreOp       = vk::AttachmentStoreOp::eDontCare;

        // The mInitialLayout should match our usage and access flags.
        auto getLayout = [](vk::ImageUsageFlags usage, AccessFlags access) {
          if (usage == vk::ImageUsageFlagBits::eColorAttachment) {
            return vk::ImageLayout::eColorAttachmentOptimal;
          }

          if (usage == vk::ImageUsageFlagBits::eInputAttachment) {
            return vk::ImageLayout::eShaderReadOnlyOptimal;
          }

          if (usage == vk::ImageUsageFlagBits::eDepthStencilAttachment) {
            if (access.containsOnly(AccessFlagBits::eLoad)) {
              return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            }
            return vk::ImageLayout::eDepthStencilAttachmentOptimal;
          }

          return vk::ImageLayout::eGeneral;
        };

        attachmentInfo.mInitialLayout = attachmentInfo.mFinalLayout = getLayout(
            passIt->mResourceUsage.at(attachment), passIt->mResourceAccess.at(attachment));

        // If there is a clear value for this resource, the AttachmentLoadOp should be eClear.
        if (Core::Utils::contains(passIt->mResourceClear, attachment)) {
          attachmentInfo.mLoadOp = vk::AttachmentLoadOp::eClear;
        } else {

          // Else if the first subpass uses it not write-only, the AttachmentLoadOp should be eLoad.
          // Else we can leave it to be eDontCare.
          AccessFlags firstUse = AccessFlagBits::eNone;
          for (auto const& subpass : passIt->mSubpasses) {
            if (Core::Utils::contains(subpass.mPass->mResourceAccess, attachment)) {
              firstUse = subpass.mPass->mResourceAccess.at(attachment);
              break;
            }
          }

          if (!firstUse.containsOnly(AccessFlagBits::eWrite)) {
            attachmentInfo.mLoadOp = vk::AttachmentLoadOp::eLoad;
          }
        }

        // If there is a later RenderPass which uses this resource, we have to make sure that the
        // final layout matches the usage in this later pass. In this case, the AttachmentStoreOp
        // should be eStore.
        if (passIt != perFrame.mRenderPasses.end()) {
          auto nextPass = passIt;
          while (++nextPass != perFrame.mRenderPasses.end()) {
            auto nextAccess = nextPass->mResourceAccess.find(attachment);
            auto nextUsage  = nextPass->mResourceUsage.find(attachment);

            if (nextAccess != nextPass->mResourceAccess.end()) {
              attachmentInfo.mFinalLayout = getLayout(nextUsage->second, nextAccess->second);
              attachmentInfo.mStoreOp     = vk::AttachmentStoreOp::eStore;
            }
          }
        }

        // If the resource is to be copied to the swapchain image, the final layout is expected to
        // be eColorAttachmentOptimal. In this case, the AttachmentStoreOp should be eStore.
        if (attachment == mOutputAttachment && passIt->mSubpasses.back().mPass == mOutputPass) {
          attachmentInfo.mFinalLayout = vk::ImageLayout::eColorAttachmentOptimal;
          attachmentInfo.mStoreOp     = vk::AttachmentStoreOp::eStore;
        }

        Core::Logger::debug() << "    \"" << attachment->mName << "\"" << std::endl;
        Core::Logger::debug() << "      InitialLayout: "
                              << vk::to_string(attachmentInfo.mInitialLayout) << std::endl;
        Core::Logger::debug() << "      FinalLayout:   "
                              << vk::to_string(attachmentInfo.mFinalLayout) << std::endl;
        Core::Logger::debug() << "      LoadOp:        " << vk::to_string(attachmentInfo.mLoadOp)
                              << std::endl;
        Core::Logger::debug() << "      StoreOp:       " << vk::to_string(attachmentInfo.mStoreOp)
                              << std::endl;

        auto image = perFrame.mAllAttachments.find(attachment);
        if (image == perFrame.mAllAttachments.end()) {
          vk::ImageAspectFlags aspect;

          if (Utils::isDepthOnlyFormat(attachment->mFormat)) {
            aspect |= vk::ImageAspectFlagBits::eDepth;
          } else if (Utils::isDepthStencilFormat(attachment->mFormat)) {
            aspect |= vk::ImageAspectFlagBits::eDepth;
            aspect |= vk::ImageAspectFlagBits::eStencil;
          } else {
            aspect |= vk::ImageAspectFlagBits::eColor;
          }

          vk::ImageCreateInfo imageInfo;
          imageInfo.imageType     = vk::ImageType::e2D;
          imageInfo.format        = attachment->mFormat;
          imageInfo.extent.width  = attachment->mExtent.x;
          imageInfo.extent.height = attachment->mExtent.y;
          imageInfo.extent.depth  = 1;
          imageInfo.mipLevels     = 1;
          imageInfo.arrayLayers   = 1;
          imageInfo.samples       = attachment->mSamples;
          imageInfo.tiling        = vk::ImageTiling::eOptimal;
          imageInfo.usage         = overallResourceUsage.at(attachment);
          imageInfo.sharingMode   = vk::SharingMode::eExclusive;
          imageInfo.initialLayout = vk::ImageLayout::eUndefined;

          perFrame.mAllAttachments[attachment] =
              mDevice->createBackedImage("Attachment \"" + attachment->mName + "\" of " + getName(),
                  imageInfo, vk::ImageViewType::e2D, aspect,
                  vk::MemoryPropertyFlagBits::eDeviceLocal, attachmentInfo.mInitialLayout);
          image = perFrame.mAllAttachments.find(attachment);
        }

        attachmentInfo.mImage = image->second;

        passIt->mRenderPass->addAttachment(attachmentInfo);
      }

      // And set the subpass info structures.
      passIt->mRenderPass->setSubpasses(subpasses);
    }

    // Create a secondary CommandBuffer for each RenderPass ----------------------------------------

    // Since Passes can be recorded in parallel, we need separate secondary CommandBuffers for each
    // one.
    for (auto& pass : perFrame.mRenderPasses) {
      for (auto& subpass : pass.mSubpasses) {
        subpass.mSecondaryCommandBuffer = CommandBuffer::create(
            subpass.mPass->mName, mDevice, QueueType::eGeneric, vk::CommandBufferLevel::eSecondary);
      }
    }

    // We are done! A new frame graph has been constructed.
    perFrame.mDirty = false;
    Core::Logger::debug() << "Frame graph construction done." << std::endl;
  }

  // -------------------------------------------------------------------------------------------- //
  // ------------------------------------ recording phase --------------------------------------- //
  // -------------------------------------------------------------------------------------------- //

  // Now we can finally start recording our command buffer. First we reset and begin our primary
  // CommandBuffer. At the very beginning of this process() method we waited for the
  // FrameFinishedFence, so we are sure that the CommandBuffer is not in use anymore.
  perFrame.mPrimaryCommandBuffer->reset();
  perFrame.mPrimaryCommandBuffer->begin();

  // A thread count of zero will make use of all available cores.
  if (flags.contains(ProcessingFlagBits::eParallelSubpassRecording)) {
    mThreadPool.setThreadCount(0);
  } else {
    mThreadPool.setThreadCount(1);
  }

  // We loop through all RenderPasses. First, we will collect the clear values for each attachment
  // and begin our RenderPass. For each subpass of each RenderPass we record the secondary
  // CommandBuffer either in parallel or sequentially.
  for (auto const& pass : perFrame.mRenderPasses) {

    // Collect clear value for each attachment.
    std::vector<vk::ClearValue> clearValues;
    for (auto attachment : pass.mAttachments) {
      if (pass.mResourceClear.find(attachment) != pass.mResourceClear.end()) {
        clearValues.push_back(pass.mResourceClear.at(attachment));
      } else {
        clearValues.push_back({});
      }
    }

    // Begin the RenderPass.
    perFrame.mPrimaryCommandBuffer->beginRenderPass(
        pass.mRenderPass, clearValues, vk::SubpassContents::eSecondaryCommandBuffers);

    // Record the subpasses using the thread(s) of the ThreadPool.
    uint32_t subpassCounter = 0;
    for (auto const& subpass : pass.mSubpasses) {
      mThreadPool.enqueue([pass, subpass, subpassCounter]() {
        vk::CommandBufferInheritanceInfo info;
        info.subpass     = subpassCounter;
        info.renderPass  = *pass.mRenderPass->getHandle();
        info.framebuffer = *pass.mRenderPass->getFramebuffer();
        subpass.mSecondaryCommandBuffer->begin(info);
        subpass.mPass->mProcessCallback(subpass.mSecondaryCommandBuffer);
        subpass.mSecondaryCommandBuffer->end();
      });
      ++subpassCounter;
    }

    // Wait until all passes are recorded.
    mThreadPool.waitIdle();

    // Execute all passes in our primary CommandBuffer.
    subpassCounter = 0;
    for (auto const& subpass : pass.mSubpasses) {
      perFrame.mPrimaryCommandBuffer->execute(subpass.mSecondaryCommandBuffer);

      if (++subpassCounter < pass.mSubpasses.size()) {
        perFrame.mPrimaryCommandBuffer->nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);
      }
    }

    // End this RenderPass.
    perFrame.mPrimaryCommandBuffer->endRenderPass();
  }

  // End and submit our primary CommandBuffer.
  perFrame.mPrimaryCommandBuffer->end();
  perFrame.mPrimaryCommandBuffer->submit({}, {}, {perFrame.mRenderFinishedSemaphore});

  // And finally present the output attachment on the output window as soon as the
  // mRenderFinishedSemaphore gets signaled.
  mOutputWindow->present(perFrame.mAllAttachments[mOutputAttachment],
      perFrame.mRenderFinishedSemaphore, perFrame.mFrameFinishedFence);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameGraph::isDirty() const {
  if (mDirty) {
    return true;
  }

  for (auto const& resource : mResources) {
    if (resource.mDirty) {
      return true;
    }
  }

  for (auto const& pass : mPasses) {
    if (pass.mDirty) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::clearDirty() {
  mDirty = false;

  for (auto& resource : mResources) {
    resource.mDirty = false;
  }

  for (auto& pass : mPasses) {
    pass.mDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::validate() const {

  Core::Logger::debug() << "Validating frame graph ..." << std::endl;

  // Check whether each resource of each pass was actually created by this frame graph.
  for (auto const& pass : mPasses) {
    for (auto const& passResource : pass.mResourceAccess) {
      bool ours = false;
      for (auto const& graphResource : mResources) {
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

  if (!mOutputAttachment) {
    throw std::runtime_error("There is no output resource set!");
  }

  // Check whether the output pass actually belongs to this graph.
  bool isOurOutputPass = false;
  for (auto const& pass : mPasses) {
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
  for (auto const& resource : mOutputPass->mResourceAccess) {
    if (mOutputAttachment == resource.first) {
      isOurOutputResource = true;
      break;
    }
  }
  if (!isOurOutputResource) {
    throw std::runtime_error("Output resource \"" + mOutputAttachment->mName +
                             "\" does not belong to output pass \"" + mOutputPass->mName +
                             "\".. Did you accidentally create a copy of the reference?");
  }

  // Check whether the resolutions of all attachments of each pass are the same
  glm::ivec2 windowExtent = mOutputWindow->pExtent.get();
  for (auto const& pass : mPasses) {
    glm::ivec2 passExtent = glm::ivec2(-1);
    for (auto const& resource : pass.mResourceAccess) {
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
  for (auto const& pass : mPasses) {
    if (!pass.mProcessCallback) {
      throw std::runtime_error("Pass \"" + pass.mName + "\" has no process-callback set!");
    }
  }

  Core::Logger::debug() << "  all good." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics

////////////////////////////////////////////////////////////////////////////////////////////////////

Illusion::Graphics::FrameGraph::ProcessingFlags operator|(
    Illusion::Graphics::FrameGraph::ProcessingFlagBits bit0,
    Illusion::Graphics::FrameGraph::ProcessingFlagBits bit1) {
  return Illusion::Graphics::FrameGraph::ProcessingFlags(bit0) | bit1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Illusion::Graphics::FrameGraph::AccessFlags operator|(
    Illusion::Graphics::FrameGraph::AccessFlagBits bit0,
    Illusion::Graphics::FrameGraph::AccessFlagBits bit1) {
  return Illusion::Graphics::FrameGraph::AccessFlags(bit0) | bit1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
