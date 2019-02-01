////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RenderPass.hpp"

#include "../Core/Logger.hpp"
#include "BackedImage.hpp"
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
  if (mDirty) {
    mDevice->waitIdle();

    mFramebuffer.reset();
    mRenderPass.reset();

    createRenderPass();
    createFramebuffer();

    mDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::addColorAttachment(Attachment const& attachment) {
  // Make sure that extent is the same.
  if (mAttachments.size() > 0 &&
      attachment.mImage->mImageInfo.extent != mAttachments[0].mImage->mImageInfo.extent) {
    throw std::runtime_error("Failed to add attachment to RenderPass \"" + getName() +
                             "\": Extent does not match previously added attachment!");
  }

  mAttachments.push_back(attachment);
  mDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::addDepthAttachment(Attachment const& attachment) {
  if (hasDepthAttachment()) {
    throw std::runtime_error("Failed to add attachment to RenderPass \"" + getName() +
                             "\": RenderPass already has a depth attachment!");
  }

  addColorAttachment(attachment);

  mDepthAttachment = mAttachments.size() - 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::clearAttachments() {
  mDepthAttachment = -1;
  mAttachments.clear();
  mDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderPass::hasDepthAttachment() const {
  return mDepthAttachment >= 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<RenderPass::Attachment> const& RenderPass::getAttachments() const {
  return mAttachments;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setSubpasses(std::vector<Subpass> const& subpasses) {
  mSubpasses = subpasses;
  mDirty     = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<RenderPass::Subpass> const& RenderPass::getSubpasses() {
  return mSubpasses;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 RenderPass::getExtent() const {
  if (mAttachments.size() > 0) {
    return glm::uvec2(mAttachments[0].mImage->mImageInfo.extent.width,
        mAttachments[0].mImage->mImageInfo.extent.height);
  }

  return glm::uvec2(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::FramebufferPtr const& RenderPass::getFramebuffer() const {
  return mFramebuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::RenderPassPtr const& RenderPass::getHandle() const {
  return mRenderPass;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::createRenderPass() {

  std::vector<vk::AttachmentDescription> attachments;
  std::vector<vk::AttachmentReference>   attachmentRefs;

  for (auto const& a : mAttachments) {

    vk::AttachmentDescription attachment;
    attachment.format        = a.mImage->mImageInfo.format;
    attachment.samples       = a.mImage->mImageInfo.samples;
    attachment.initialLayout = a.mInitialLayout;
    attachment.finalLayout   = a.mFinalLayout;
    attachment.loadOp        = a.mLoadOp;
    attachment.storeOp       = a.mStoreOp;

    vk::AttachmentReference attachmentRef;
    attachmentRef.layout     = a.mFinalLayout;
    attachmentRef.attachment = static_cast<uint32_t>(attachments.size());

    attachments.emplace_back(attachment);
    attachmentRefs.emplace_back(attachmentRef);
  }

  auto& subpasses = mSubpasses;

  // create default subpass if none are specified
  std::vector<Subpass> defaultSubpass(1);

  if (subpasses.size() == 0) {
    for (size_t i(0); i < attachments.size(); ++i) {
      defaultSubpass[0].mOutputAttachments.push_back(i);
    }

    subpasses = defaultSubpass;
  }

  std::vector<vk::SubpassDescription>               subpassInfos(subpasses.size());
  std::vector<std::vector<vk::AttachmentReference>> inputAttachmentRefs(subpasses.size());
  std::vector<std::vector<vk::AttachmentReference>> outputAttachmentRefs(subpasses.size());

  for (size_t i(0); i < subpasses.size(); ++i) {

    for (uint32_t attachment : subpasses[i].mInputAttachments) {
      inputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
    }

    for (auto attachment : subpasses[i].mOutputAttachments) {
      if (int32_t(attachment) == mDepthAttachment) {
        subpassInfos[i].pDepthStencilAttachment = &attachmentRefs[mDepthAttachment];
      } else {
        outputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
      }
    }

    subpassInfos[i].pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subpassInfos[i].inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRefs[i].size());
    subpassInfos[i].pInputAttachments    = inputAttachmentRefs[i].data();
    subpassInfos[i].colorAttachmentCount = static_cast<uint32_t>(outputAttachmentRefs[i].size());
    subpassInfos[i].pColorAttachments    = outputAttachmentRefs[i].data();
  }

  std::vector<vk::SubpassDependency> dependencies;
  for (size_t dst(0); dst < subpasses.size(); ++dst) {
    for (auto src : subpasses[dst].mPreSubpasses) {
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
  info.subpassCount    = static_cast<uint32_t>(subpassInfos.size());
  info.pSubpasses      = subpassInfos.data();
  info.dependencyCount = static_cast<uint32_t>(dependencies.size());
  info.pDependencies   = dependencies.data();

  mRenderPass = mDevice->createRenderPass(getName(), info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::createFramebuffer() {

  std::vector<vk::ImageView> imageViews(mAttachments.size());

  for (size_t i(0); i < mAttachments.size(); ++i) {
    imageViews[i] = *mAttachments[i].mImage->mView;
  }

  vk::FramebufferCreateInfo info;
  info.renderPass      = *mRenderPass;
  info.attachmentCount = static_cast<uint32_t>(imageViews.size());
  info.pAttachments    = imageViews.data();
  info.width           = mAttachments[0].mImage->mImageInfo.extent.width;
  info.height          = mAttachments[0].mImage->mImageInfo.extent.height;
  info.layers          = 1;

  mFramebuffer = mDevice->createFramebuffer(getName(), info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
