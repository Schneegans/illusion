////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LazyRenderPass.hpp"

#include "Utils.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

LazyRenderPass::LazyRenderPass(std::string const& name, DeviceConstPtr const& device)
    : RenderPass(name, device) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

LazyRenderPass::~LazyRenderPass() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////

void LazyRenderPass::init() {
  if (mDirty) {
    clearAttachments();
    createImages();

    RenderPass::init();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void LazyRenderPass::addAttachment(vk::Format format) {
  mAttachmentFormats.push_back(format);
  mDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void LazyRenderPass::setExtent(glm::uvec2 const& extent) {
  if (mExtent != extent) {
    mExtent = extent;
    mDirty  = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void LazyRenderPass::createImages() {
  for (size_t i(0); i < mAttachmentFormats.size(); ++i) {
    vk::ImageAspectFlags aspect;

    if (Utils::isDepthOnlyFormat(mAttachmentFormats[i])) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
    } else if (Utils::isDepthStencilFormat(mAttachmentFormats[i])) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
      aspect |= vk::ImageAspectFlagBits::eStencil;
    } else {
      aspect |= vk::ImageAspectFlagBits::eColor;
    }

    vk::ImageUsageFlags usage;
    vk::ImageLayout     layout;

    if (Utils::isColorFormat(mAttachmentFormats[i])) {
      usage  = vk::ImageUsageFlagBits::eColorAttachment;
      layout = vk::ImageLayout::eColorAttachmentOptimal;
    } else {
      usage  = vk::ImageUsageFlagBits::eDepthStencilAttachment;
      layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    // eTransferSrc is actually only required for the attachment which will be blitted to the
    // swapchain images
    usage |= vk::ImageUsageFlagBits::eTransferSrc;

    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.format        = mAttachmentFormats[i];
    imageInfo.extent.width  = mExtent.x;
    imageInfo.extent.height = mExtent.y;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.samples       = vk::SampleCountFlagBits::e1;
    imageInfo.tiling        = vk::ImageTiling::eOptimal;
    imageInfo.usage         = usage;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    Attachment attachment;
    attachment.mInitialLayout = layout;
    attachment.mFinalLayout   = layout;
    attachment.mLoadOp        = vk::AttachmentLoadOp::eClear;
    attachment.mStoreOp       = vk::AttachmentStoreOp::eStore;
    attachment.mImage         = mDevice->createBackedImage(
        "Attachment " + std::to_string(i) + " of " + getName(), imageInfo, vk::ImageViewType::e2D,
        aspect, vk::MemoryPropertyFlagBits::eDeviceLocal, layout);

    RenderPass::addAttachment(attachment);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
