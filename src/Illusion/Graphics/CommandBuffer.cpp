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
#include "DescriptorSet.hpp"
#include "Device.hpp"
#include "PipelineReflection.hpp"
#include "RenderPass.hpp"
#include "ShaderProgram.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

CommandBuffer::CommandBuffer(
  std::shared_ptr<Device> const& device, QueueType type, vk::CommandBufferLevel level)
  : mDevice(device)
  , mVkCmd(device->allocateCommandBuffer(type, level))
  , mType(type)
  , mLevel(level)
  , mDescriptorSetCache(device)
  , mGraphicsState(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::reset() const { mVkCmd->reset({}); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::begin(vk::CommandBufferUsageFlagBits usage) const { mVkCmd->begin({usage}); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::end() const { mVkCmd->end(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::submit(
  std::vector<vk::Semaphore> const&          waitSemaphores,
  std::vector<vk::PipelineStageFlags> const& waitStages,
  std::vector<vk::Semaphore> const&          signalSemaphores,
  vk::Fence const&                           fence) const {

  vk::CommandBuffer bufs[] = {*mVkCmd};

  vk::SubmitInfo info;
  info.pWaitDstStageMask    = waitStages.data();
  info.commandBufferCount   = 1;
  info.pCommandBuffers      = bufs;
  info.signalSemaphoreCount = signalSemaphores.size();
  info.pSignalSemaphores    = signalSemaphores.data();
  info.waitSemaphoreCount   = waitSemaphores.size();
  info.pWaitSemaphores      = waitSemaphores.data();

  mDevice->getQueue(mType).submit(info, fence);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::waitIdle() const { mDevice->getQueue(mType).waitIdle(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::beginRenderPass(std::shared_ptr<RenderPass> const& renderPass) {
  renderPass->init();

  vk::RenderPassBeginInfo passInfo;
  passInfo.renderPass               = *renderPass->getHandle();
  passInfo.framebuffer              = *renderPass->getFramebuffer()->getHandle();
  passInfo.renderArea.offset        = vk::Offset2D(0, 0);
  passInfo.renderArea.extent.width  = renderPass->getExtent().x;
  passInfo.renderArea.extent.height = renderPass->getExtent().y;

  std::vector<vk::ClearValue> clearValues;
  clearValues.push_back(vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}}));

  if (renderPass->hasDepthAttachment()) {
    clearValues.push_back(vk::ClearDepthStencilValue(1.f, 0.f));
  }

  passInfo.clearValueCount = clearValues.size();
  passInfo.pClearValues    = clearValues.data();

  mVkCmd->beginRenderPass(passInfo, vk::SubpassContents::eInline);

  mCurrentRenderPass = renderPass;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::endRenderPass() {
  mVkCmd->endRenderPass();
  mCurrentRenderPass.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindIndexBuffer(
  vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType indexType) const {
  mVkCmd->bindIndexBuffer(buffer, offset, indexType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindVertexBuffers(
  uint32_t                             firstBinding,
  vk::ArrayProxy<const vk::Buffer>     buffers,
  vk::ArrayProxy<const vk::DeviceSize> offsets) const {
  mVkCmd->bindVertexBuffers(firstBinding, buffers, offsets);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindCombinedImageSampler(
  std::shared_ptr<Texture> const& texture, uint32_t set, uint32_t binding) const {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::draw(
  uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {

  flush();
  mVkCmd->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::drawIndexed(
  uint32_t indexCount,
  uint32_t instanceCount,
  uint32_t firstIndex,
  int32_t  vertexOffset,
  uint32_t firstInstance) {

  flush();
  mVkCmd->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsState& CommandBuffer::graphicsState() { return mGraphicsState; }

////////////////////////////////////////////////////////////////////////////////////////////////////

BindingState& CommandBuffer::bindingState() { return mBindingState; }

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

  mVkCmd->pipelineBarrier(stage, stage, vk::DependencyFlagBits(), nullptr, nullptr, barrier);
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

  mVkCmd->copyImage(
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

  mVkCmd->blitImage(
    src,
    vk::ImageLayout::eTransferSrcOptimal,
    dst,
    vk::ImageLayout::eTransferDstOptimal,
    info,
    filter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::resolveImage(
  vk::Image        src,
  vk::ImageLayout  srcLayout,
  vk::Image        dst,
  vk::ImageLayout  dstLayout,
  vk::ImageResolve region) const {
  mVkCmd->resolveImage(src, srcLayout, dst, dstLayout, region);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const {
  vk::BufferCopy region;
  region.size = size;

  mVkCmd->copyBuffer(src, dst, 1, &region);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::copyBufferToImage(
  vk::Buffer                              src,
  vk::Image                               dst,
  vk::ImageLayout                         dstLayout,
  std::vector<vk::BufferImageCopy> const& infos) const {

  mVkCmd->copyBufferToImage(src, dst, dstLayout, infos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::flush() {
  auto pipeline = mGraphicsState.getPipelineHandle(mCurrentRenderPass, 0);
  mVkCmd->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

  for (uint32_t set : mBindingState.getDirtySets()) {

    // auto descriptorSet = mDescriptorSetCache.acquireHandle(
    //   mGraphicsState.getShaderProgram()->getDescriptorSetReflections().at(set));
    // // descriptorSet->bindCombinedImageSampler(texture, 0);

    // mVkCmd->bindDescriptorSets(
    //   vk::PipelineBindPoint::eGraphics,
    //   *mGraphicsState.getShaderProgram()->getReflection()->getLayout(),
    //   set,
    //   *descriptorSet,
    //   {});
  }

  mBindingState.clearDirtySets();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
