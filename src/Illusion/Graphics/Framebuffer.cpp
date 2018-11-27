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
#include "Framebuffer.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "Utils.hpp"

#include <iostream>

namespace Illusion::Graphics {
////////////////////////////////////////////////////////////////////////////////////////////////////

Framebuffer::Framebuffer(
  std::shared_ptr<Device> const&         device,
  std::shared_ptr<vk::RenderPass> const& renderPass,
  glm::uvec2 const&                      extent,
  std::vector<vk::Format> const&         attachments)
  : mDevice(device)
  , mRenderPass(renderPass)
  , mExtent(extent) {

  ILLUSION_TRACE << "Creating Framebuffer." << std::endl;

  for (auto attachment : attachments) {
    vk::ImageAspectFlags aspect;

    if (Utils::isDepthOnlyFormat(attachment)) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
    } else if (Utils::isDepthStencilFormat(attachment)) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
      aspect |= vk::ImageAspectFlagBits::eStencil;
    } else {
      aspect |= vk::ImageAspectFlagBits::eColor;
    }

    // eTransferSrc is actually only required for the attachment which will be blitted to the
    // swapchain images
    vk::ImageUsageFlags usage =
      vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

    if (Utils::isDepthFormat(attachment)) {
      usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    }

    auto image = mDevice->createBackedImage(
      extent.x,
      extent.y,
      1,
      1,
      1,
      attachment,
      vk::ImageTiling::eOptimal,
      usage,
      vk::MemoryPropertyFlagBits::eDeviceLocal,
      vk::SampleCountFlagBits::e1);

    vk::ImageViewCreateInfo info;
    info.image                           = *image->mImage;
    info.viewType                        = vk::ImageViewType::e2D;
    info.format                          = attachment;
    info.components.r                    = vk::ComponentSwizzle::eIdentity;
    info.components.g                    = vk::ComponentSwizzle::eIdentity;
    info.components.b                    = vk::ComponentSwizzle::eIdentity;
    info.components.a                    = vk::ComponentSwizzle::eIdentity;
    info.subresourceRange.aspectMask     = aspect;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;

    mImageStore.push_back(image);
    mImageViewStore.push_back(mDevice->createImageView(info));
  }

  std::vector<vk::ImageView> imageViewInfos(mImageViewStore.size());

  for (size_t i{0}; i < mImageViewStore.size(); ++i) {
    imageViewInfos[i] = *mImageViewStore[i];
  }

  vk::FramebufferCreateInfo info;
  info.renderPass      = *mRenderPass;
  info.attachmentCount = imageViewInfos.size();
  info.pAttachments    = imageViewInfos.data();
  info.width           = mExtent.x;
  info.height          = mExtent.y;
  info.layers          = 1;

  mFramebuffer = mDevice->createFramebuffer(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Framebuffer::~Framebuffer() { ILLUSION_TRACE << "Deleting Framebuffer." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
