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
#include <utility>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::RenderPass(std::string const& name, DeviceConstPtr device)
    : Core::NamedObject(name)
    , mDevice(std::move(device)) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::~RenderPass() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::init() {
  if (mDirty) {
    mFramebuffer.reset();
    mRenderPass.reset();

    createRenderPass();
    createFramebuffer();

    mDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 RenderPass::getExtent() const {
  if (!mAttachments.empty()) {
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

void RenderPass::addAttachment(Attachment const& attachment) {
  // Make sure that extent is the same.
  if (!mAttachments.empty() &&
      attachment.mImage->mImageInfo.extent != mAttachments[0].mImage->mImageInfo.extent) {
    throw std::runtime_error("Failed to add attachment to RenderPass \"" + getName() +
                             "\": Extent does not match previously added attachment!");
  }

  mAttachments.push_back(attachment);
  mDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setAttachments(std::vector<Attachment> const& attachments) {
  mAttachments = attachments;
  mDirty       = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<RenderPass::Attachment> const& RenderPass::getAttachments() const {
  return mAttachments;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::clearAttachments() {
  mAttachments.clear();
  mDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::addSubpass(Subpass const& subpass) {
  mSubpasses.push_back(subpass);
  mDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setSubpasses(std::vector<Subpass> const& subpasses) {
  mSubpasses = subpasses;
  mDirty     = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<RenderPass::Subpass> const& RenderPass::getSubpasses() const {
  return mSubpasses;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::clearSubpasses() {
  mSubpasses.clear();
  mDirty = true;
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

  if (subpasses.empty()) {
    for (size_t i(0); i < attachments.size(); ++i) {
      if (Utils::isColorFormat(attachments[i].format)) {
        defaultSubpass[0].mColorAttachments.push_back(i);
      } else {
        defaultSubpass[0].mDepthStencilAttachment = i;
      }
    }

    subpasses = defaultSubpass;
  }

  std::vector<vk::SubpassDescription>               subpassInfos(subpasses.size());
  std::vector<std::vector<vk::AttachmentReference>> inputAttachmentRefs(subpasses.size());
  std::vector<std::vector<vk::AttachmentReference>> colorAttachmentRefs(subpasses.size());

  for (size_t i(0); i < subpasses.size(); ++i) {

    for (uint32_t attachment : subpasses[i].mInputAttachments) {
      inputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
    }

    for (uint32_t attachment : subpasses[i].mColorAttachments) {
      colorAttachmentRefs[i].push_back(attachmentRefs[attachment]);
    }

    if (subpasses[i].mDepthStencilAttachment) {
      subpassInfos[i].pDepthStencilAttachment =
          &attachmentRefs[*subpasses[i].mDepthStencilAttachment];
    }

    subpassInfos[i].pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subpassInfos[i].inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRefs[i].size());
    subpassInfos[i].pInputAttachments    = inputAttachmentRefs[i].data();
    subpassInfos[i].colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs[i].size());
    subpassInfos[i].pColorAttachments    = colorAttachmentRefs[i].data();
  }

  std::vector<vk::SubpassDependency> dependencies;
  for (size_t dst(0); dst < subpasses.size(); ++dst) {
    for (auto src : subpasses[dst].mPreSubpasses) {
      vk::SubpassDependency dependency;
      dependency.srcSubpass    = src;
      dependency.dstSubpass    = static_cast<uint32_t>(dst);
      dependency.srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
      dependency.dstStageMask  = vk::PipelineStageFlagBits::eFragmentShader;
      dependency.dstAccessMask = vk::AccessFlagBits::eInputAttachmentRead;
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
