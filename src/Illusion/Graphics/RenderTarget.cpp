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
#include "RenderTarget.hpp"

#include "../Core/Logger.hpp"
#include "Context.hpp"
#include "Utils.hpp"

#include <iostream>

namespace Illusion::Graphics {
////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTarget::RenderTarget(
  std::shared_ptr<Context> const&           context,
  std::shared_ptr<vk::RenderPass> const&    renderPass,
  vk::Extent2D const&                       extent,
  std::vector<AttachmentDescription> const& attachmentDescriptions)
  : mContext(context)
  , mRenderPass(renderPass)
  , mExtent(extent) {

  ILLUSION_TRACE << "Creating RenderTarget." << std::endl;

  for (auto attachment : attachmentDescriptions) {
    vk::ImageAspectFlags aspect;

    if (Utils::isDepthOnlyFormat(attachment.mFormat)) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
    } else if (Utils::isDepthStencilFormat(attachment.mFormat)) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
      aspect |= vk::ImageAspectFlagBits::eStencil;
    } else {
      aspect |= vk::ImageAspectFlagBits::eColor;
    }

    vk::ImageViewCreateInfo info;
    info.image                           = *attachment.mImage;
    info.viewType                        = vk::ImageViewType::e2D;
    info.format                          = attachment.mFormat;
    info.components.r                    = vk::ComponentSwizzle::eIdentity;
    info.components.g                    = vk::ComponentSwizzle::eIdentity;
    info.components.b                    = vk::ComponentSwizzle::eIdentity;
    info.components.a                    = vk::ComponentSwizzle::eIdentity;
    info.subresourceRange.aspectMask     = aspect;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;

    mImageStore.push_back(attachment.mImage);
    mImageViewStore.push_back(mContext->createImageView(info));
  }

  std::vector<vk::ImageView> attachments(mImageViewStore.size());

  for (size_t i{0}; i < mImageViewStore.size(); ++i) {
    attachments[i] = *mImageViewStore[i];
  }

  vk::FramebufferCreateInfo info;
  info.renderPass      = *mRenderPass;
  info.attachmentCount = attachments.size();
  info.pAttachments    = attachments.data();
  info.width           = mExtent.width;
  info.height          = mExtent.height;
  info.layers          = 1;

  mFramebuffer = mContext->createFramebuffer(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTarget::~RenderTarget() { ILLUSION_TRACE << "Deleting RenderTarget." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
