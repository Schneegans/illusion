////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RenderPass.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "PhysicalDevice.hpp"
#include "Utils.hpp"
#include "Window.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::RenderPass(std::string const& name, DevicePtr const& device)
    : Core::NamedObject(name)
    , mDevice(device) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::~RenderPass() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::init() {
  if (mAttachmentsDirty) {
    mDevice->waitIdle();

    mFramebuffer.reset();
    mRenderPass.reset();

    mRenderPass  = createRenderPass();
    mFramebuffer = std::make_shared<Framebuffer>("Framebuffer of " + getName(), mDevice,
        mRenderPass, mExtent, mFrameBufferAttachmentFormats);

    mAttachmentsDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::addAttachment(vk::Format format) {
  mFrameBufferAttachmentFormats.push_back(format);
  mAttachmentsDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setSubPasses(std::vector<SubPass> const& subPasses) {
  mSubPasses        = subPasses;
  mAttachmentsDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setExtent(glm::uvec2 const& extent) {
  if (mExtent != extent) {
    mExtent           = extent;
    mAttachmentsDirty = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 const& RenderPass::getExtent() const {
  return mExtent;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FramebufferPtr const& RenderPass::getFramebuffer() const {
  return mFramebuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::RenderPassPtr const& RenderPass::getHandle() const {
  return mRenderPass;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderPass::hasDepthAttachment() const {
  for (auto format : mFrameBufferAttachmentFormats) {
    if (Utils::isDepthFormat(format))
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<vk::Format> const& RenderPass::getFrameBufferAttachmentFormats() const {
  return mFrameBufferAttachmentFormats;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::RenderPassPtr RenderPass::createRenderPass() const {

  std::vector<vk::AttachmentDescription> attachments;
  std::vector<vk::AttachmentReference>   attachmentRefs;
  int                                    depthStencilAttachmentRef(-1);

  for (size_t i(0); i < mFrameBufferAttachmentFormats.size(); ++i) {
    vk::AttachmentDescription attachment;
    vk::AttachmentReference   attachmentRef;

    attachment.format        = mFrameBufferAttachmentFormats[i];
    attachment.samples       = vk::SampleCountFlagBits::e1;
    attachment.initialLayout = vk::ImageLayout::eUndefined;
    attachment.loadOp        = vk::AttachmentLoadOp::eClear;
    attachment.storeOp       = vk::AttachmentStoreOp::eStore;

    if (Utils::isColorFormat(attachment.format)) {
      attachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
      attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      attachment.finalLayout    = vk::ImageLayout::eColorAttachmentOptimal;
    } else if (Utils::isDepthOnlyFormat(attachment.format)) {
      attachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
      attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      attachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    } else {
      attachment.stencilLoadOp  = vk::AttachmentLoadOp::eClear;
      attachment.stencilStoreOp = vk::AttachmentStoreOp::eStore;
      attachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    attachmentRef.layout     = attachment.finalLayout;
    attachmentRef.attachment = static_cast<uint32_t>(attachments.size());

    attachments.emplace_back(attachment);
    attachmentRefs.emplace_back(attachmentRef);

    if (Utils::isDepthFormat(attachment.format)) {
      depthStencilAttachmentRef = attachmentRef.attachment;
    }
  }

  // create default subpass if none are specified
  if (mSubPasses.size() == 0) {
    std::vector<vk::AttachmentReference> colorAttachmentRefs;
    for (size_t i(0); i < attachmentRefs.size(); ++i) {
      if ((int)i != depthStencilAttachmentRef) {
        colorAttachmentRefs.push_back(attachmentRefs[i]);
      }
    }

    vk::SubpassDescription subPass;
    subPass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subPass.pColorAttachments    = colorAttachmentRefs.data();

    if (depthStencilAttachmentRef >= 0) {
      subPass.pDepthStencilAttachment = &attachmentRefs[depthStencilAttachmentRef];
    }

    vk::RenderPassCreateInfo info;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments    = attachments.data();
    info.subpassCount    = 1;
    info.pSubpasses      = &subPass;

    return mDevice->createRenderPass(getName(), info);
  }

  std::vector<vk::SubpassDescription>               subPasses(mSubPasses.size());
  std::vector<std::vector<vk::AttachmentReference>> inputAttachmentRefs(mSubPasses.size());
  std::vector<std::vector<vk::AttachmentReference>> outputAttachmentRefs(mSubPasses.size());

  for (size_t i(0); i < mSubPasses.size(); ++i) {

    for (uint32_t attachment : mSubPasses[i].mInputAttachments) {
      inputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
    }

    for (auto attachment : mSubPasses[i].mOutputAttachments) {
      if ((int)attachment == depthStencilAttachmentRef) {
        subPasses[i].pDepthStencilAttachment = &attachmentRefs[depthStencilAttachmentRef];
      } else {
        outputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
      }
    }

    subPasses[i].pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subPasses[i].inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRefs[i].size());
    subPasses[i].pInputAttachments    = inputAttachmentRefs[i].data();
    subPasses[i].colorAttachmentCount = static_cast<uint32_t>(outputAttachmentRefs[i].size());
    subPasses[i].pColorAttachments    = outputAttachmentRefs[i].data();
  }

  std::vector<vk::SubpassDependency> dependencies;
  for (size_t dst(0); dst < mSubPasses.size(); ++dst) {
    for (auto src : mSubPasses[dst].mPreSubPasses) {
      vk::SubpassDependency dependency;
      dependency.srcSubpass    = src;
      dependency.dstSubpass    = static_cast<uint32_t>(dst);
      dependency.srcStageMask  = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependency.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependency.dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependency.dstAccessMask =
          vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies.emplace_back(dependency);
    }
  }

  vk::RenderPassCreateInfo info;
  info.attachmentCount = static_cast<uint32_t>(attachments.size());
  info.pAttachments    = attachments.data();
  info.subpassCount    = static_cast<uint32_t>(subPasses.size());
  info.pSubpasses      = subPasses.data();
  info.dependencyCount = static_cast<uint32_t>(dependencies.size());
  info.pDependencies   = dependencies.data();

  return mDevice->createRenderPass(getName(), info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
