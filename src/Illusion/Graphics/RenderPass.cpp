////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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

RenderPass::RenderPass(std::shared_ptr<Device> const& device)
  : mDevice(device)
  , mPipelineCache(device) {

  ILLUSION_TRACE << "Creating RenderPass." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::~RenderPass() { ILLUSION_TRACE << "Deleting RenderPass." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::init() {
  if (mAttachmentsDirty) {
    mDevice->waitIdle();

    mFramebuffer.reset();
    mRenderPass.reset();
    mPipelineCache.clear();

    mRenderPass = createRenderPass();
    mFramebuffer =
      std::make_shared<Framebuffer>(mDevice, mRenderPass, mExtent, mFrameBufferAttachmentFormats);

    mAttachmentsDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::begin(std::shared_ptr<CommandBuffer> const& cmd) {
  init();

  vk::RenderPassBeginInfo passInfo;
  passInfo.renderPass               = *mRenderPass;
  passInfo.framebuffer              = *mFramebuffer->getFramebuffer();
  passInfo.renderArea.offset        = vk::Offset2D(0, 0);
  passInfo.renderArea.extent.width  = mExtent.x;
  passInfo.renderArea.extent.height = mExtent.y;

  std::vector<vk::ClearValue> clearValues;
  clearValues.push_back(vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}}));

  if (hasDepthAttachment()) { clearValues.push_back(vk::ClearDepthStencilValue(1.f, 0.f)); }

  passInfo.clearValueCount = clearValues.size();
  passInfo.pClearValues    = clearValues.data();

  cmd->beginRenderPass(passInfo, vk::SubpassContents::eInline);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::end(std::shared_ptr<CommandBuffer> const& cmd) { cmd->endRenderPass(); }

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

std::shared_ptr<vk::Pipeline> RenderPass::getPipelineHandle(
  GraphicsState const& graphicsState, uint32_t subPass) {
  init();

  return mPipelineCache.getPipelineHandle(graphicsState, *mRenderPass, subPass);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setExtent(glm::uvec2 const& extent) {
  if (mExtent != extent) {
    mExtent           = extent;
    mAttachmentsDirty = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 const& RenderPass::getExtent() const { return mExtent; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Framebuffer> const& RenderPass::getFramebuffer() const { return mFramebuffer; }

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderPass::hasDepthAttachment() const {
  for (auto format : mFrameBufferAttachmentFormats) {
    if (Utils::isDepthFormat(format)) return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::RenderPass> RenderPass::createRenderPass() const {

  std::vector<vk::AttachmentDescription> attachments;
  std::vector<vk::AttachmentReference>   attachmentRefs;
  int                                    depthStencilAttachmentRef{-1};

  for (size_t i{0}; i < mFrameBufferAttachmentFormats.size(); ++i) {
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
    attachmentRef.attachment = attachments.size();

    attachments.emplace_back(attachment);
    attachmentRefs.emplace_back(attachmentRef);

    if (Utils::isDepthFormat(attachment.format)) {
      depthStencilAttachmentRef = attachmentRef.attachment;
    }
  }

  // create default subpass if none are specified
  if (mSubPasses.size() == 0) {
    std::vector<vk::AttachmentReference> colorAttachmentRefs;
    for (int i{0}; i < (int)attachmentRefs.size(); ++i) {
      if (i != depthStencilAttachmentRef) { colorAttachmentRefs.push_back(attachmentRefs[i]); }
    }

    vk::SubpassDescription subPass;
    subPass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subPass.colorAttachmentCount = colorAttachmentRefs.size();
    subPass.pColorAttachments    = colorAttachmentRefs.data();

    if (depthStencilAttachmentRef >= 0) {
      subPass.pDepthStencilAttachment = &attachmentRefs[depthStencilAttachmentRef];
    }

    vk::RenderPassCreateInfo info;
    info.attachmentCount = attachments.size();
    info.pAttachments    = attachments.data();
    info.subpassCount    = 1;
    info.pSubpasses      = &subPass;

    return mDevice->createRenderPass(info);
  }

  std::vector<vk::SubpassDescription>               subPasses(mSubPasses.size());
  std::vector<std::vector<vk::AttachmentReference>> inputAttachmentRefs(mSubPasses.size());
  std::vector<std::vector<vk::AttachmentReference>> outputAttachmentRefs(mSubPasses.size());

  for (size_t i{0}; i < mSubPasses.size(); ++i) {

    for (uint32_t attachment : mSubPasses[i].mInputAttachments) {
      inputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
    }

    for (int attachment : mSubPasses[i].mOutputAttachments) {
      if (attachment == depthStencilAttachmentRef) {
        subPasses[i].pDepthStencilAttachment = &attachmentRefs[depthStencilAttachmentRef];
      } else {
        outputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
      }
    }

    subPasses[i].pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subPasses[i].inputAttachmentCount = inputAttachmentRefs[i].size();
    subPasses[i].pInputAttachments    = inputAttachmentRefs[i].data();
    subPasses[i].colorAttachmentCount = outputAttachmentRefs[i].size();
    subPasses[i].pColorAttachments    = outputAttachmentRefs[i].data();
  }

  std::vector<vk::SubpassDependency> dependencies;
  for (size_t dst{0}; dst < mSubPasses.size(); ++dst) {
    for (auto src : mSubPasses[dst].mPreSubPasses) {
      vk::SubpassDependency dependency;
      dependency.srcSubpass    = src;
      dependency.dstSubpass    = dst;
      dependency.srcStageMask  = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependency.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependency.dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependency.dstAccessMask =
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies.emplace_back(dependency);
    }
  }

  vk::RenderPassCreateInfo info;
  info.attachmentCount = attachments.size();
  info.pAttachments    = attachments.data();
  info.subpassCount    = subPasses.size();
  info.pSubpasses      = subPasses.data();
  info.dependencyCount = dependencies.size();
  info.pDependencies   = dependencies.data();

  return mDevice->createRenderPass(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
