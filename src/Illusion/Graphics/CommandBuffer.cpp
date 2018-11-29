////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CommandBuffer.hpp"

#include "../Core/Logger.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

CommandBuffer::CommandBuffer(vk::CommandBuffer const& base)
  : vk::CommandBuffer(base) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(
  vk::Image                 image,
  vk::ImageLayout           oldLayout,
  vk::ImageLayout           newLayout,
  vk::PipelineStageFlagBits stage,
  vk::ImageSubresourceRange range) const {

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout           = oldLayout;
  barrier.newLayout           = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image               = image;
  barrier.subresourceRange    = range;

  pipelineBarrier(stage, stage, vk::DependencyFlagBits(), nullptr, nullptr, barrier);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::copyImage(vk::Image src, vk::Image dst, glm::uvec2 const& size) const {

  vk::ImageSubresourceLayers subResource;
  subResource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  subResource.baseArrayLayer = 0;
  subResource.mipLevel       = 0;
  subResource.layerCount     = 1;

  vk::ImageCopy region;
  region.srcSubresource = subResource;
  region.dstSubresource = subResource;
  region.srcOffset      = vk::Offset3D(0, 0, 0);
  region.dstOffset      = vk::Offset3D(0, 0, 0);
  region.extent.width   = size.x;
  region.extent.height  = size.y;
  region.extent.depth   = 1;

  vk::CommandBuffer::copyImage(
    src, vk::ImageLayout::eTransferSrcOptimal, dst, vk::ImageLayout::eTransferDstOptimal, region);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::blitImage(
  vk::Image         src,
  vk::Image         dst,
  glm::uvec2 const& srcSize,
  glm::uvec2 const& dstSize,
  vk::Filter        filter) const {

  vk::ImageBlit info;
  info.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
  info.srcSubresource.layerCount = 1;
  info.srcOffsets[0]             = vk::Offset3D(0, 0, 0);
  info.srcOffsets[1]             = vk::Offset3D(srcSize.x, srcSize.y, 1);
  info.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
  info.dstSubresource.layerCount = 1;
  info.dstOffsets[0]             = vk::Offset3D(0, 0, 0);
  info.dstOffsets[1]             = vk::Offset3D(dstSize.x, dstSize.y, 1);

  vk::CommandBuffer::blitImage(
    src,
    vk::ImageLayout::eTransferSrcOptimal,
    dst,
    vk::ImageLayout::eTransferDstOptimal,
    info,
    filter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const {
  vk::BufferCopy region;
  region.size = size;
  vk::CommandBuffer::copyBuffer(src, dst, 1, &region);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
