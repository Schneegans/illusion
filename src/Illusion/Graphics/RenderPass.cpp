////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "RenderPass.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "PhysicalDevice.hpp"
#include "Utils.hpp"
#include "Window.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::RenderPass(std::shared_ptr<Context> const& context)
  : mContext(context)
  , mPipelineFactory(context) {

  ILLUSION_TRACE << "Creating RenderPass." << std::endl;

  mSignalSemaphore = createSignalSemaphore();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::~RenderPass() { ILLUSION_TRACE << "Deleting RenderPass." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::init() {
  if (mRingbufferSizeDirty) {
    mContext->getDevice()->waitIdle();

    mFences.clear();
    mCommandBuffers.clear();

    mFences         = createFences();
    mCommandBuffers = createCommandBuffers();

    mRingbufferSizeDirty = false;
    mAttachmentsDirty    = true;
  }

  if (mAttachmentsDirty) {
    mContext->getDevice()->waitIdle();

    mRenderTargets.clear();
    mFrameBufferAttachments.clear();

    mRenderPass.reset();
    mPipelineFactory.clearCache();

    mRenderPass             = createRenderPass();
    mFrameBufferAttachments = createFramebufferAttachments();
    mRenderTargets          = createRenderTargets();

    mAttachmentsDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::render() {
  init();

  mContext->getDevice()->waitForFences(*mFences[mCurrentRingBufferIndex], true, ~0);

  auto cmd = mCommandBuffers[mCurrentRingBufferIndex];
  mContext->getDevice()->resetFences(*mFences[mCurrentRingBufferIndex]);

  // record command buffer -------------------------------------------------------------------------

  cmd->reset(vk::CommandBufferResetFlags());

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
  cmd->begin(beginInfo);

  // record render pass ----------------------------------------------------------------------------
  if (beforeFunc) { beforeFunc(cmd, *this); }

  vk::RenderPassBeginInfo passInfo;
  passInfo.renderPass        = *mRenderPass;
  passInfo.framebuffer       = *mRenderTargets[mCurrentRingBufferIndex]->getFramebuffer();
  passInfo.renderArea.offset = vk::Offset2D(0, 0);
  passInfo.renderArea.extent = mExtent;

  std::vector<vk::ClearValue> clearValues;
  clearValues.push_back(vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}}));

  if (hasDepthAttachment()) { clearValues.push_back(vk::ClearDepthStencilValue(1.f, 0.f)); }

  passInfo.clearValueCount = clearValues.size();
  passInfo.pClearValues    = clearValues.data();

  cmd->beginRenderPass(passInfo, vk::SubpassContents::eInline);

  if (drawFunc) { drawFunc(cmd, *this, 0); }

  if (mSubPasses.size() > 0) {
    for (size_t i{1}; i < mSubPasses.size(); ++i) {
      cmd->nextSubpass(vk::SubpassContents::eInline);
      drawFunc(cmd, *this, i);
    }
  }

  cmd->endRenderPass();

  // submit to queue -------------------------------------------------------------------------------

  cmd->end();

  vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

  std::vector<vk::Semaphore> waitSemaphores(mWaitSemaphores.size());
  for (uint32_t i{0}; i < mWaitSemaphores.size(); ++i) {
    waitSemaphores[i] = *mWaitSemaphores[i].lock();
  }

  vk::Semaphore signalSemaphores[] = {*mSignalSemaphore};

  vk::SubmitInfo submitInfo;
  submitInfo.waitSemaphoreCount   = waitSemaphores.size();
  submitInfo.pWaitSemaphores      = waitSemaphores.data();
  submitInfo.pWaitDstStageMask    = waitStages;
  submitInfo.commandBufferCount   = 1;
  submitInfo.pCommandBuffers      = cmd.get();
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  mContext->getGraphicsQueue().submit(submitInfo, *mFences[mCurrentRingBufferIndex]);
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

vk::Pipeline const& RenderPass::getPipelineHandle(
  GraphicsState const& graphicsState, uint32_t subPass) const {
  if (!mRenderPass) {
    throw std::runtime_error("Cannot create pipeline: Please initialize the RenderPass first!");
  }

  return mPipelineFactory.getPipelineHandle(graphicsState, *mRenderPass, subPass);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setExtent(vk::Extent2D const& extent) {
  if (mExtent != extent) {
    mExtent           = extent;
    mAttachmentsDirty = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Extent2D const& RenderPass::getExtent() const { return mExtent; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setRingBufferSize(uint32_t size) {
  if (mRingbufferSize != size) {
    mRingbufferSize      = size;
    mRingbufferSizeDirty = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t RenderPass::getRingBufferSize() const { return mRingbufferSize; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::executeAfter(std::shared_ptr<RenderPass> const& other) {
  mWaitSemaphores.push_back(other->mSignalSemaphore);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::executeBefore(std::shared_ptr<RenderPass> const& other) {
  other->mWaitSemaphores.push_back(mSignalSemaphore);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderPass::hasDepthAttachment() const {
  for (auto format : mFrameBufferAttachmentFormats) {
    if (Utils::isDepthFormat(format)) return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setSwapchainInfo(std::vector<vk::Image> const& images, vk::Format format) {
  mSwapchainImages  = images;
  mSwapchainFormat  = format;
  mAttachmentsDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Semaphore> RenderPass::createSignalSemaphore() const {
  vk::SemaphoreCreateInfo info;
  return mContext->createSemaphore(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<vk::Fence>> RenderPass::createFences() const {
  std::vector<std::shared_ptr<vk::Fence>> fences;
  for (uint32_t i = 0; i < mRingbufferSize; ++i) {
    vk::FenceCreateInfo info;
    info.flags = vk::FenceCreateFlagBits::eSignaled;
    fences.push_back(mContext->createFence(info));
  }
  return fences;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<CommandBuffer>> RenderPass::createCommandBuffers() const {
  std::vector<std::shared_ptr<CommandBuffer>> commandBuffers(mRingbufferSize);

  for (int i(0); i < mRingbufferSize; ++i) {
    commandBuffers[i] = mContext->allocateGraphicsCommandBuffer();
  }

  return commandBuffers;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::RenderPass> RenderPass::createRenderPass() const {

  std::vector<vk::AttachmentDescription> attachments;
  std::vector<vk::AttachmentReference>   attachmentRefs;
  int                                    depthStencilAttachmentRef{-1};

  if (mSwapchainImages.size() > 0) {
    vk::AttachmentDescription attachment;
    vk::AttachmentReference   attachmentRef;

    attachment.format         = mSwapchainFormat;
    attachment.samples        = vk::SampleCountFlagBits::e1;
    attachment.loadOp         = vk::AttachmentLoadOp::eClear;
    attachment.storeOp        = vk::AttachmentStoreOp::eStore;
    attachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachment.initialLayout  = vk::ImageLayout::eUndefined;
    attachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;
    attachmentRef.attachment  = 0;
    attachmentRef.layout      = vk::ImageLayout::eColorAttachmentOptimal;

    attachments.emplace_back(attachment);
    attachmentRefs.emplace_back(attachmentRef);
  }

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

    return mContext->createRenderPass(info);
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

  return mContext->createRenderPass(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<BackedImage>> RenderPass::createFramebufferAttachments() const {
  std::vector<std::shared_ptr<BackedImage>> attachments(mFrameBufferAttachmentFormats.size());

  for (size_t i{0}; i < mFrameBufferAttachmentFormats.size(); ++i) {

    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;

    if (Utils::isDepthFormat(mFrameBufferAttachmentFormats[i])) {
      usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    }

    attachments[i] = mContext->createBackedImage(
      mExtent.width,
      mExtent.height,
      1,
      1,
      1,
      mFrameBufferAttachmentFormats[i],
      vk::ImageTiling::eOptimal,
      usage,
      vk::MemoryPropertyFlagBits::eDeviceLocal,
      vk::SampleCountFlagBits::e1);
  }

  return attachments;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<RenderTarget>> RenderPass::createRenderTargets() const {
  std::vector<std::shared_ptr<RenderTarget>> renderTargets;

  for (size_t i{0}; i < mRingbufferSize; ++i) {
    std::vector<RenderTarget::AttachmentDescription> attachments;

    if (mSwapchainImages.size() > 0) {
      attachments.push_back({mSwapchainFormat, std::make_shared<vk::Image>(mSwapchainImages[i])});
    }

    for (size_t j{0}; j < mFrameBufferAttachmentFormats.size(); ++j) {
      attachments.push_back({mFrameBufferAttachmentFormats[j], mFrameBufferAttachments[j]->mImage});
    }

    renderTargets.emplace_back(
      std::make_shared<RenderTarget>(mContext, mRenderPass, mExtent, attachments));
  }

  return renderTargets;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
