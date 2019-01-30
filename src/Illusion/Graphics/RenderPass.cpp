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
  if (mAttachmentsDirty) {
    mDevice->waitIdle();

    mFramebuffer.reset();
    mRenderPass.reset();

    createRenderPass();
    createFramebuffer();

    mAttachmentsDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::addAttachment(BackedImagePtr const& image) {
  // Make sure that extent is the same.
  if (mImageStore.size() > 0 && image->mImageInfo.extent != mImageStore[0]->mImageInfo.extent) {
    throw std::runtime_error("Failed to add attachment to RenderPass \"" + getName() +
                             "\": Extent does not match previously added attachment!");
  }

  mImageStore.push_back(image);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::setSubpasses(std::vector<SubpassInfo> const& subpasses) {
  mSubpasses        = subpasses;
  mAttachmentsDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 RenderPass::getExtent() const {
  if (mImageStore.size() > 0) {
    return glm::uvec2(
        mImageStore[0]->mImageInfo.extent.width, mImageStore[0]->mImageInfo.extent.height);
  }

  return glm::uvec2(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<BackedImagePtr> const& RenderPass::getAttachments() const {
  return mImageStore;
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

bool RenderPass::hasDepthAttachment() const {
  for (auto image : mImageStore) {
    if (Utils::isDepthFormat(image->mImageInfo.format))
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::createRenderPass() {

  std::vector<vk::AttachmentDescription> attachments;
  std::vector<vk::AttachmentReference>   attachmentRefs;
  int                                    depthStencilAttachmentRef(-1);

  for (auto const& image : mImageStore) {
    vk::AttachmentDescription attachment;
    vk::AttachmentReference   attachmentRef;

    attachment.format        = image->mImageInfo.format;
    attachment.samples       = image->mImageInfo.samples;
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
  if (mSubpasses.size() == 0) {
    std::vector<vk::AttachmentReference> colorAttachmentRefs;
    for (size_t i(0); i < attachmentRefs.size(); ++i) {
      if ((int)i != depthStencilAttachmentRef) {
        colorAttachmentRefs.push_back(attachmentRefs[i]);
      }
    }

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments    = colorAttachmentRefs.data();

    if (depthStencilAttachmentRef >= 0) {
      subpass.pDepthStencilAttachment = &attachmentRefs[depthStencilAttachmentRef];
    }

    vk::RenderPassCreateInfo info;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments    = attachments.data();
    info.subpassCount    = 1;
    info.pSubpasses      = &subpass;

    mRenderPass = mDevice->createRenderPass(getName(), info);
    return;
  }

  std::vector<vk::SubpassDescription>               subpasses(mSubpasses.size());
  std::vector<std::vector<vk::AttachmentReference>> inputAttachmentRefs(mSubpasses.size());
  std::vector<std::vector<vk::AttachmentReference>> outputAttachmentRefs(mSubpasses.size());

  for (size_t i(0); i < mSubpasses.size(); ++i) {

    for (uint32_t attachment : mSubpasses[i].mInputAttachments) {
      inputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
    }

    for (auto attachment : mSubpasses[i].mOutputAttachments) {
      if ((int)attachment == depthStencilAttachmentRef) {
        subpasses[i].pDepthStencilAttachment = &attachmentRefs[depthStencilAttachmentRef];
      } else {
        outputAttachmentRefs[i].push_back(attachmentRefs[attachment]);
      }
    }

    subpasses[i].pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subpasses[i].inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRefs[i].size());
    subpasses[i].pInputAttachments    = inputAttachmentRefs[i].data();
    subpasses[i].colorAttachmentCount = static_cast<uint32_t>(outputAttachmentRefs[i].size());
    subpasses[i].pColorAttachments    = outputAttachmentRefs[i].data();
  }

  std::vector<vk::SubpassDependency> dependencies;
  for (size_t dst(0); dst < mSubpasses.size(); ++dst) {
    for (auto src : mSubpasses[dst].mPreSubpasses) {
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
  info.subpassCount    = static_cast<uint32_t>(subpasses.size());
  info.pSubpasses      = subpasses.data();
  info.dependencyCount = static_cast<uint32_t>(dependencies.size());
  info.pDependencies   = dependencies.data();

  mRenderPass = mDevice->createRenderPass(getName(), info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::createFramebuffer() {

  std::vector<vk::ImageView> imageViews(mImageStore.size());

  for (size_t i(0); i < mImageStore.size(); ++i) {
    imageViews[i] = *mImageStore[i]->mView;
  }

  vk::FramebufferCreateInfo info;
  info.renderPass      = *mRenderPass;
  info.attachmentCount = static_cast<uint32_t>(imageViews.size());
  info.pAttachments    = imageViews.data();
  info.width           = mImageStore[0]->mImageInfo.extent.width;
  info.height          = mImageStore[0]->mImageInfo.extent.height;
  info.layers          = 1;

  mFramebuffer = mDevice->createFramebuffer(getName(), info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
