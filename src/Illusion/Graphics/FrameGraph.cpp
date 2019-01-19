////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FrameGraph.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"

#include <iostream>
#include <queue>
#include <unordered_set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setName(std::string const& name) {
  mName  = name;
  mDirty = true;
  return *this;
}

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setFormat(vk::Format format) {
  mFormat = format;
  mDirty  = true;
  return *this;
}

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setType(Type type) {
  mType  = type;
  mDirty = true;
  return *this;
}

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setSizing(Sizing sizing) {
  mSizing = sizing;
  mDirty  = true;
  return *this;
}

FrameGraph::LogicalResource& FrameGraph::LogicalResource::setExtent(glm::uvec2 const& extent) {
  mExtent = extent;
  mDirty  = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::LogicalPass::addResource(
    LogicalResource const& resource, ResourceUsage usage, std::optional<vk::ClearValue> clear) {

  if (mLogicalResources.find(&resource) != mLogicalResources.end()) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to frame graph pass \"" + mName +
                             "\": Resource has already been added to this pass!");
  }
  mLogicalResources[&resource] = {usage, clear};
  mDirty                       = true;
  return *this;
}

FrameGraph::LogicalPass& FrameGraph::LogicalPass::setName(std::string const& name) {
  mName  = name;
  mDirty = true;
  return *this;
}

FrameGraph::LogicalPass& FrameGraph::LogicalPass::addInputAttachment(
    LogicalResource const& resource) {
  return addResource(resource, ResourceUsage::eInputAttachment);
}

FrameGraph::LogicalPass& FrameGraph::LogicalPass::addBlendAttachment(
    LogicalResource const& resource) {
  return addResource(resource, ResourceUsage::eBlendAttachment);
}

FrameGraph::LogicalPass& FrameGraph::LogicalPass::addOutputAttachment(
    LogicalResource const& resource, std::optional<vk::ClearValue> clear) {
  return addResource(resource, ResourceUsage::eOutputAttachment, clear);
}

FrameGraph::LogicalPass& FrameGraph::LogicalPass::setOutputWindow(WindowPtr const& window) {
  mOutputWindow = window;
  mDirty        = true;
  return *this;
}

FrameGraph::LogicalPass& FrameGraph::LogicalPass::setProcessCallback(
    std::function<void(CommandBufferPtr)> const& callback) {
  mProcessCallback = callback;
  mDirty           = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::FrameGraph(DevicePtr const& device, FrameResourceIndexPtr const& frameIndex)
    : mDevice(device)
    , mPerFrame(frameIndex, [device]() {
      PerFrame perFrame;
      perFrame.mPrimaryCommandBuffer    = CommandBuffer::create(device);
      perFrame.mRenderFinishedSemaphore = device->createSemaphore();
      perFrame.mFrameFinishedFence      = device->createFence();
      return perFrame;
    }) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalResource& FrameGraph::addResource() {
  mDirty = true;
  mLogicalResources.push_back({});
  return mLogicalResources.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameGraph::LogicalPass& FrameGraph::addPass() {
  mDirty = true;
  mLogicalPasses.push_back({});
  return mLogicalPasses.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameGraph::process() {

  if (isDirty()) {

    // validate resources, in- and outputs
    try {
      validate();
    } catch (std::runtime_error e) {
      throw std::runtime_error("frame graph validation failed: " + std::string(e.what()));
    }

    // make sure that we re-create all render passes
    for (auto& perFrame : mPerFrame) {
      perFrame.mDirty = true;
    }

    clearDirty();
  }

  auto& perFrame = mPerFrame.current();

  mDevice->waitForFence(perFrame.mFrameFinishedFence);
  mDevice->resetFence(perFrame.mFrameFinishedFence);

  if (perFrame.mDirty) {
    ILLUSION_MESSAGE << "Recreate!" << std::endl;
    perFrame.mDirty = false;
  }

  perFrame.mPrimaryCommandBuffer->reset();
  perFrame.mPrimaryCommandBuffer->begin();

  // perFrame.mPrimaryCommandBuffer->beginRenderPass(res.mRenderPass);

  std::for_each(mLogicalPasses.begin(), mLogicalPasses.end(),
      [perFrame](auto const& pass) { pass.mProcessCallback(perFrame.mPrimaryCommandBuffer); });

  // perFrame.mPrimaryCommandBuffer->endRenderPass();

  perFrame.mPrimaryCommandBuffer->end();

  perFrame.mPrimaryCommandBuffer->submit({}, {}, {perFrame.mRenderFinishedSemaphore});

  // window->present(mRenderPass->getFramebuffer()->getImages()[0],
  // perFrame.mRenderFinishedSemaphore,
  //     perFrame.mFrameFinishedFence);
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

  // Check whether each resource of each pass was actually created by this frame graph
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

  // Check whether each resource is used in the graph and that each resource's first use is as
  // output attachment
  for (auto const& resource : mLogicalResources) {
    bool used = false;
    for (auto& pass : mLogicalPasses) {
      auto passResource = pass.mLogicalResources.find(&resource);
      if (passResource != pass.mLogicalResources.end()) {
        if (passResource->second.mUsage != LogicalPass::ResourceUsage::eOutputAttachment) {
          throw std::runtime_error(
              "First use of resource \"" + resource.mName + "\" must be output attachment!");
        }
        used = true;
        break;
      }
    }

    if (!used) {
      throw std::runtime_error("Resource \"" + resource.mName + "\" is not used at all!");
    }
  }

  // Check whether we have exactly one pass with an output window
  uint32_t outputWindows = 0;
  for (auto const& pass : mLogicalPasses) {
    if (pass.mOutputWindow) {
      ++outputWindows;
    }
  }

  if (outputWindows != 1) {
    throw std::runtime_error("There must be exactly one output window in the graph.");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
