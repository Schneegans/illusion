////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CommandBuffer.hpp"

#include "../Core/Logger.hpp"
#include "BackedBuffer.hpp"
#include "Device.hpp"
#include "PipelineReflection.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "ShaderModule.hpp"
#include "Texture.hpp"

#include <iostream>
#include <utility>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

CommandBuffer::CommandBuffer(std::string const& name, DeviceConstPtr const& device, QueueType type,
    vk::CommandBufferLevel level)
    : Core::NamedObject(name)
    , mDevice(device)
    , mVkCmd(device->allocateCommandBuffer(name, type, level))
    , mType(type)
    , mLevel(level)
    , mGraphicsState(device)
    , mDescriptorSetCache(name, device) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::setMaxPipelineAge(uint64_t value) {
  mMaxPipelineAge = value;
}

uint64_t CommandBuffer::getMaxPipelineAge() const {
  return mMaxPipelineAge;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::reset() {

  // First clear all state of the CommandBuffer. Except for the GraphicsState, this is kept.
  mBindingState.reset();
  mSpecialisationState.reset();
  mCurrentDescriptorSets.clear();
  mDescriptorSetCache.releaseAll();
  mCurrentRenderPass.reset();
  mCurrentSubpass = 0;

  // Increment our recording counter. This is used to track the life time of pipeline cache entries.
  ++mRecordingID;

  // Now delete all pipelines which are older than mMaxPipelineAge.
  auto it = mPipelineCache.begin();
  while (it != mPipelineCache.end()) {
    if (mRecordingID - it->second.second > mMaxPipelineAge) {
      it = mPipelineCache.erase(it);
    } else {
      ++it;
    }
  }

  // Then do the actual vk::CommandBuffer resetting.
  mVkCmd->reset({});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::begin(vk::CommandBufferUsageFlagBits usage) {
  mVkCmd->begin({usage});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::begin(RenderPassPtr const& currentRenderPass, uint32_t currentSubpass,
    vk::CommandBufferUsageFlagBits usage) {

  currentRenderPass->init();

  mCurrentRenderPass = currentRenderPass;
  mCurrentSubpass    = currentSubpass;

  vk::CommandBufferInheritanceInfo info;
  info.subpass     = mCurrentSubpass;
  info.renderPass  = *mCurrentRenderPass->getHandle();
  info.framebuffer = *mCurrentRenderPass->getFramebuffer();

  mVkCmd->begin({usage, &info});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::end() {
  mCurrentRenderPass.reset();
  mCurrentSubpass = 0;
  mVkCmd->end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::submit(std::vector<vk::SemaphorePtr> const& waitSemaphores,
    std::vector<vk::PipelineStageFlags> const&                  waitStages,
    std::vector<vk::SemaphorePtr> const& signalSemaphores, vk::FencePtr const& fence) const {

  vk::CommandBuffer bufs[] = {*mVkCmd};

  // As all Semaphores are passed in as std::shared_ptr's, we have to put them dereferenced into
  // temporary vectors.

  std::vector<vk::Semaphore> tmpWaitSemaphores(waitSemaphores.size());
  for (size_t i(0); i < waitSemaphores.size(); ++i) {
    tmpWaitSemaphores[i] = *waitSemaphores[i];
  }

  std::vector<vk::Semaphore> tmpSignalSemaphores(signalSemaphores.size());
  for (size_t i(0); i < signalSemaphores.size(); ++i) {
    tmpSignalSemaphores[i] = *signalSemaphores[i];
  }

  vk::SubmitInfo info;
  info.pWaitDstStageMask    = waitStages.data();
  info.commandBufferCount   = 1;
  info.pCommandBuffers      = bufs;
  info.signalSemaphoreCount = static_cast<uint32_t>(tmpSignalSemaphores.size());
  info.pSignalSemaphores    = tmpSignalSemaphores.data();
  info.waitSemaphoreCount   = static_cast<uint32_t>(tmpWaitSemaphores.size());
  info.pWaitSemaphores      = tmpWaitSemaphores.data();

  // Submit to the queue which was defined at contruction time.
  mDevice->getQueue(mType).submit(info, fence ? *fence : nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::waitIdle() const {
  mDevice->getQueue(mType).waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::beginRenderPass(RenderPassPtr const& renderPass,
    std::vector<vk::ClearValue> const& clearValues, vk::SubpassContents contents) {
  renderPass->init();

  vk::RenderPassBeginInfo passInfo;
  passInfo.renderPass               = *renderPass->getHandle();
  passInfo.framebuffer              = *renderPass->getFramebuffer();
  passInfo.renderArea.offset        = vk::Offset2D(0, 0);
  passInfo.renderArea.extent.width  = renderPass->getExtent().x;
  passInfo.renderArea.extent.height = renderPass->getExtent().y;
  passInfo.clearValueCount          = static_cast<uint32_t>(clearValues.size());
  passInfo.pClearValues             = clearValues.data();

  mVkCmd->beginRenderPass(passInfo, contents);

  // Store a pointer to the currently active RenderPass. This is required for later construction of
  // the Pipelines.
  mCurrentRenderPass = renderPass;
  mCurrentSubpass    = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::endRenderPass() {
  mVkCmd->endRenderPass();

  // There is no currently active RenderPass anymore.
  mCurrentRenderPass.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::execute(CommandBufferPtr const& secondary) {
  mVkCmd->executeCommands(*secondary->mVkCmd);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::execute(std::vector<CommandBufferPtr> const& secondaries) {
  std::vector<vk::CommandBuffer> cmds(secondaries.size());

  for (size_t i(0); i < secondaries.size(); ++i) {
    cmds[i] = *secondaries[i]->mVkCmd;
  }

  mVkCmd->executeCommands(cmds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::nextSubpass(vk::SubpassContents contents) {
  ++mCurrentSubpass;
  mVkCmd->nextSubpass(contents);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsState& CommandBuffer::graphicsState() {
  return mGraphicsState;
}
GraphicsState const& CommandBuffer::graphicsState() const {
  return mGraphicsState;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BindingState& CommandBuffer::bindingState() {
  return mBindingState;
}
BindingState const& CommandBuffer::bindingState() const {
  return mBindingState;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SpecialisationState& CommandBuffer::specialisationState() {
  return mSpecialisationState;
}
SpecialisationState const& CommandBuffer::specialisationState() const {
  return mSpecialisationState;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::setShader(ShaderPtr const& val) {
  mCurrentShader = val;
}
ShaderPtr const& CommandBuffer::getShader() const {
  return mCurrentShader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindIndexBuffer(
    BackedBufferPtr const& buffer, vk::DeviceSize offset, vk::IndexType indexType) const {
  mVkCmd->bindIndexBuffer(*buffer->mBuffer, offset, indexType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindVertexBuffers(uint32_t                     firstBinding,
    std::vector<std::pair<BackedBufferPtr, vk::DeviceSize>> const& buffersAndOffsets) const {

  std::vector<vk::Buffer>     buffers;
  std::vector<vk::DeviceSize> offsets;

  for (auto const& v : buffersAndOffsets) {
    buffers.emplace_back(*v.first->mBuffer);
    offsets.emplace_back(v.second);
  }

  mVkCmd->bindVertexBuffers(firstBinding, buffers, offsets);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindVertexBuffers(
    uint32_t firstBinding, std::vector<BackedBufferPtr> const& buffs) const {

  std::vector<vk::Buffer>     buffers;
  std::vector<vk::DeviceSize> offsets;

  for (auto const& v : buffs) {
    buffers.emplace_back(*v->mBuffer);
    offsets.emplace_back(0uL);
  }

  mVkCmd->bindVertexBuffers(firstBinding, buffers, offsets);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::pushConstants(const void* data, uint32_t size, uint32_t offset) const {

  if (!mCurrentShader) {
    throw std::runtime_error("Failed to set push constants: There must be an active Shader!");
  }

  auto const& reflection = mCurrentShader->getReflection();
  auto constants = reflection->getResources(PipelineResource::ResourceType::ePushConstantBuffer);

  if (constants.size() != 1) {
    throw std::runtime_error("Failed to set push constants: There must be exactly one "
                             "PushConstantBuffer defined in the pipeline reflection!");
  }

  mVkCmd->pushConstants(
      *reflection->getLayout(), constants.begin()->second.mStages, offset, size, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::draw(
    uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {

  // First, bind a Pipeline and create, update and bind DescriptorSets.
  flush();

  // The record the actual draw call.
  mVkCmd->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
    int32_t vertexOffset, uint32_t firstInstance) {

  // First, bind a Pipeline and create, update and bind DescriptorSets.
  flush();

  // The record the actual draw call.
  mVkCmd->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {

  // First, bind a Pipeline and create, update and bind DescriptorSets.
  flush();

  // The record the actual dispatch call.
  mVkCmd->dispatch(groupCountX, groupCountY, groupCountZ);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout,
    vk::AccessFlags const& srcAccess, vk::PipelineStageFlagBits srcStage, vk::ImageLayout newLayout,
    vk::AccessFlags const& dstAccess, vk::PipelineStageFlagBits dstStage,
    vk::ImageSubresourceRange range) const {

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout           = oldLayout;
  barrier.newLayout           = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image               = image;
  barrier.subresourceRange    = std::move(range);
  barrier.srcAccessMask       = srcAccess;
  barrier.dstAccessMask       = dstAccess;

  mVkCmd->pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits(), nullptr, nullptr, barrier);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout, vk::ImageSubresourceRange range) const {
  // clang-format off
  static const std::unordered_map<vk::ImageLayout, vk::PipelineStageFlagBits> srcStageMapping = {
    {vk::ImageLayout::eUndefined,                     vk::PipelineStageFlagBits::eTopOfPipe},
    {vk::ImageLayout::ePreinitialized,                vk::PipelineStageFlagBits::eTopOfPipe},
    {vk::ImageLayout::eGeneral,                       vk::PipelineStageFlagBits::eColorAttachmentOutput},
    {vk::ImageLayout::eColorAttachmentOptimal,        vk::PipelineStageFlagBits::eColorAttachmentOutput},
    {vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits::eLateFragmentTests},
    {vk::ImageLayout::eDepthStencilReadOnlyOptimal,   vk::PipelineStageFlagBits::eLateFragmentTests},
    {vk::ImageLayout::eShaderReadOnlyOptimal,         vk::PipelineStageFlagBits::eFragmentShader},
    {vk::ImageLayout::eTransferSrcOptimal,            vk::PipelineStageFlagBits::eTransfer},
    {vk::ImageLayout::eTransferDstOptimal,            vk::PipelineStageFlagBits::eTransfer},
    {vk::ImageLayout::ePresentSrcKHR,                 vk::PipelineStageFlagBits::eTransfer}
  };

  static const std::unordered_map<vk::ImageLayout, vk::PipelineStageFlagBits> dstStageMapping = {
    {vk::ImageLayout::eGeneral,                       vk::PipelineStageFlagBits::eVertexShader},
    {vk::ImageLayout::eColorAttachmentOptimal,        vk::PipelineStageFlagBits::eColorAttachmentOutput},
    {vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits::eEarlyFragmentTests},
    {vk::ImageLayout::eDepthStencilReadOnlyOptimal,   vk::PipelineStageFlagBits::eVertexShader},
    {vk::ImageLayout::eShaderReadOnlyOptimal,         vk::PipelineStageFlagBits::eVertexShader},
    {vk::ImageLayout::eTransferSrcOptimal,            vk::PipelineStageFlagBits::eTransfer},
    {vk::ImageLayout::eTransferDstOptimal,            vk::PipelineStageFlagBits::eTransfer},
    {vk::ImageLayout::ePresentSrcKHR,                 vk::PipelineStageFlagBits::eTransfer}
  };

  static const std::unordered_map<vk::ImageLayout, vk::AccessFlags> srcAccessMapping = {
    {vk::ImageLayout::eUndefined,                     vk::AccessFlags()},
    {vk::ImageLayout::ePreinitialized,                vk::AccessFlags()},
    {vk::ImageLayout::eGeneral,                       vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite},
    {vk::ImageLayout::eColorAttachmentOptimal,        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite},
    {vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite},
    {vk::ImageLayout::eDepthStencilReadOnlyOptimal,   vk::AccessFlagBits::eDepthStencilAttachmentRead},
    {vk::ImageLayout::eShaderReadOnlyOptimal,         vk::AccessFlagBits::eInputAttachmentRead},
    {vk::ImageLayout::eTransferSrcOptimal,            vk::AccessFlagBits::eTransferRead},
    {vk::ImageLayout::eTransferDstOptimal,            vk::AccessFlagBits::eTransferWrite},
    {vk::ImageLayout::ePresentSrcKHR,                 vk::AccessFlagBits::eMemoryRead}
  };

  static const std::unordered_map<vk::ImageLayout, vk::AccessFlags> dstAccessMapping = {
    {vk::ImageLayout::eGeneral,                       vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite},
    {vk::ImageLayout::eColorAttachmentOptimal,        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite},
    {vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite},
    {vk::ImageLayout::eDepthStencilReadOnlyOptimal,   vk::AccessFlagBits::eShaderRead},
    {vk::ImageLayout::eShaderReadOnlyOptimal,         vk::AccessFlagBits::eShaderRead},
    {vk::ImageLayout::eTransferSrcOptimal,            vk::AccessFlagBits::eTransferRead},
    {vk::ImageLayout::eTransferDstOptimal,            vk::AccessFlagBits::eTransferWrite},
    {vk::ImageLayout::ePresentSrcKHR,                 vk::AccessFlagBits::eMemoryRead}
  };
  // clang-format on

  auto srcStage  = srcStageMapping.find(oldLayout);
  auto srcAccess = srcAccessMapping.find(oldLayout);
  auto dstStage  = dstStageMapping.find(newLayout);
  auto dstAccess = dstAccessMapping.find(newLayout);

  if (srcStage == srcStageMapping.end() || dstStage == dstStageMapping.end() ||
      srcAccess == srcAccessMapping.end() || dstAccess == dstAccessMapping.end()) {
    throw std::runtime_error("Failed to transition image layout: Unsupported transition!");
  }

  transitionImageLayout(image, oldLayout, srcAccess->second, srcStage->second, newLayout,
      dstAccess->second, dstStage->second, std::move(range));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(BackedImagePtr const& image, vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout, vk::ImageSubresourceRange range) const {

  transitionImageLayout(*image->mImage, oldLayout, newLayout, std::move(range));
  image->mCurrentLayout = newLayout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(
    BackedImagePtr const& image, vk::ImageLayout newLayout, vk::ImageSubresourceRange range) const {
  transitionImageLayout(image, image->mCurrentLayout, newLayout, std::move(range));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(
    BackedImagePtr const& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
  transitionImageLayout(image, oldLayout, newLayout, image->mViewInfo.subresourceRange);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(
    BackedImagePtr const& image, vk::ImageLayout newLayout) const {
  transitionImageLayout(image, image->mCurrentLayout, newLayout, image->mViewInfo.subresourceRange);
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

void CommandBuffer::blitImage(vk::Image src, vk::Image dst, glm::uvec2 const& srcSize,
    glm::uvec2 const& dstSize, vk::Filter filter) const {

  blitImage(src, 0, dst, 0, srcSize, dstSize, 1, filter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::blitImage(vk::Image src, uint32_t srcMipmapLevel, vk::Image dst,
    uint32_t dstMipmapLevel, glm::uvec2 const& srcSize, glm::uvec2 const& dstSize,
    uint32_t layerCount, vk::Filter filter) const {

  vk::ImageBlit info;
  info.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
  info.srcSubresource.layerCount = layerCount;
  info.srcSubresource.mipLevel   = srcMipmapLevel;
  info.srcOffsets[0]             = vk::Offset3D(0, 0, 0);
  info.srcOffsets[1]             = vk::Offset3D(srcSize.x, srcSize.y, 1);
  info.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
  info.dstSubresource.layerCount = layerCount;
  info.dstSubresource.mipLevel   = dstMipmapLevel;
  info.dstOffsets[0]             = vk::Offset3D(0, 0, 0);
  info.dstOffsets[1]             = vk::Offset3D(dstSize.x, dstSize.y, 1);

  mVkCmd->blitImage(src, vk::ImageLayout::eTransferSrcOptimal, dst,
      vk::ImageLayout::eTransferDstOptimal, info, filter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::resolveImage(vk::Image src, vk::ImageLayout srcLayout, vk::Image dst,
    vk::ImageLayout dstLayout, vk::ImageResolve const& region) const {
  mVkCmd->resolveImage(src, srcLayout, dst, dstLayout, region);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const {
  vk::BufferCopy region;
  region.size = size;

  mVkCmd->copyBuffer(src, dst, 1, &region);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::copyBufferToImage(vk::Buffer src, vk::Image dst, vk::ImageLayout dstLayout,
    std::vector<vk::BufferImageCopy> const& infos) const {

  mVkCmd->copyBufferToImage(src, dst, dstLayout, infos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::copyImageToBuffer(vk::Image src, vk::Buffer dst, vk::ImageLayout srcLayout,
    std::vector<vk::BufferImageCopy> const& infos) const {

  mVkCmd->copyImageToBuffer(src, srcLayout, dst, infos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::flush() {

  if (!mCurrentShader) {
    throw std::runtime_error(
        "Failed to flush CommandBuffer \"" + getName() + "\": There must be an active Shader!");
  }

  vk::PipelineBindPoint bindPoint = mType == QueueType::eCompute ? vk::PipelineBindPoint::eCompute
                                                                 : vk::PipelineBindPoint::eGraphics;

  // create (or retrieve from cache) and bind a pipeline -------------------------------------------
  auto pipeline = getPipelineHandle();
  mVkCmd->bindPipeline(bindPoint, *pipeline);

  // now bind and update all DescriptorSets -------------------------------------------------------

  // the logic is roughly as follows:

  // for each DescriptorSet number of the current program
  //   if BindingState of this set number is dirty (a binding for this set has been changed)
  //     or no DescriptorSet is currently bound for this set
  //     or the layout of the currently bound DescriptorSet is incompatible to the current program
  //       acquire new DescriptorSet
  //       update DescriptorSet
  //       bind DescriptorSet
  //       store hash for compatibility checks
  //   else if a dynamic offset has been changed
  //       re-bind current DescriptorSet

  // get DescriptorSet layouts of the current program
  auto const& setReflections = mCurrentShader->getDescriptorSetReflections();

  for (uint32_t setNum = 0; setNum < setReflections.size(); ++setNum) {

    // Ignore empty DescriptorSets.
    if (setReflections[setNum]->getResources().empty()) {
      continue;
    }

    // There is nothing to bind, most likely the user forgot to bind something - but it may also be
    // on purpose when the current program actually does not need this set.
    if (mBindingState.getBindings(setNum).empty()) {
      continue;
    }

    // Get hash of the currently bound DescriptorSet (if any) to check if it is compatible with the
    // DescriptorSet layout of the currently bound program.
    auto currentSetIt = mCurrentDescriptorSets.find(setNum);

    // We need to bind a new DescriptorSet if
    //   BindingState of this set number is dirty (a binding for this set has been changed),
    //   or no DescriptorSet is currently bound for this set,
    //   or the layout of the currently bound DescriptorSet is incompatible to the current program.
    if (mBindingState.getDirtySets().find(setNum) != mBindingState.getDirtySets().end() ||
        currentSetIt == mCurrentDescriptorSets.end() ||
        currentSetIt->second.mSetLayoutHash != setReflections[setNum]->getHash()) {

      // Acquire an unused DescriptorSet.
      auto descriptorSet = mDescriptorSetCache.acquireHandle(
          mCurrentShader->getDescriptorSetReflections().at(setNum));

      // Get all bindings of the current DescriptorSet.
      auto const& bindings     = mBindingState.getBindings(setNum);
      size_t      bindingCount = bindings.size();

      // This will store the offsets of dynamic uniform and storage buffers.
      std::vector<uint32_t> dynamicOffsets;

      // Write DescriptorSet for each binding.
      vk::WriteDescriptorSet defaulWriteInfo;
      defaulWriteInfo.dstSet          = *descriptorSet;
      defaulWriteInfo.dstArrayElement = 0;
      defaulWriteInfo.descriptorCount = 1;

      std::vector<vk::WriteDescriptorSet>   writeInfos(bindingCount, defaulWriteInfo);
      std::vector<vk::DescriptorImageInfo>  imageInfos(bindingCount);
      std::vector<vk::DescriptorBufferInfo> bufferInfos(bindingCount);

      uint32_t i = 0;

      for (auto const& binding : bindings) {

        writeInfos[i].dstBinding = binding.first;

        if (std::holds_alternative<InputAttachmentBinding>(binding.second)) {

          auto value                   = std::get<InputAttachmentBinding>(binding.second);
          imageInfos[i].imageLayout    = value.mAttachment->mCurrentLayout;
          imageInfos[i].imageView      = *value.mAttachment->mView;
          writeInfos[i].descriptorType = vk::DescriptorType::eInputAttachment;
          writeInfos[i].pImageInfo     = &imageInfos[i];

        } else if (std::holds_alternative<CombinedImageSamplerBinding>(binding.second)) {

          auto value                   = std::get<CombinedImageSamplerBinding>(binding.second);
          imageInfos[i].imageLayout    = value.mTexture->mCurrentLayout;
          imageInfos[i].imageView      = *value.mTexture->mView;
          imageInfos[i].sampler        = *value.mTexture->mSampler;
          writeInfos[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
          writeInfos[i].pImageInfo     = &imageInfos[i];

        } else if (std::holds_alternative<StorageImageBinding>(binding.second)) {

          auto value                   = std::get<StorageImageBinding>(binding.second);
          imageInfos[i].imageLayout    = value.mImage->mCurrentLayout;
          imageInfos[i].imageView      = *(value.mView ? value.mView : value.mImage->mView);
          imageInfos[i].sampler        = *value.mImage->mSampler;
          writeInfos[i].descriptorType = vk::DescriptorType::eStorageImage;
          writeInfos[i].pImageInfo     = &imageInfos[i];

        } else if (std::holds_alternative<UniformBufferBinding>(binding.second)) {

          auto value                   = std::get<UniformBufferBinding>(binding.second);
          bufferInfos[i].buffer        = *value.mBuffer->mBuffer;
          bufferInfos[i].offset        = value.mOffset;
          bufferInfos[i].range         = value.mSize;
          writeInfos[i].descriptorType = vk::DescriptorType::eUniformBuffer;
          writeInfos[i].pBufferInfo    = &bufferInfos[i];

        } else if (std::holds_alternative<DynamicUniformBufferBinding>(binding.second)) {

          auto value                   = std::get<DynamicUniformBufferBinding>(binding.second);
          bufferInfos[i].buffer        = *value.mBuffer->mBuffer;
          bufferInfos[i].range         = value.mSize;
          writeInfos[i].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
          writeInfos[i].pBufferInfo    = &bufferInfos[i];

          dynamicOffsets.push_back(mBindingState.getDynamicOffset(setNum, binding.first));

        } else if (std::holds_alternative<StorageBufferBinding>(binding.second)) {

          auto value                   = std::get<StorageBufferBinding>(binding.second);
          bufferInfos[i].buffer        = *value.mBuffer->mBuffer;
          bufferInfos[i].offset        = value.mOffset;
          bufferInfos[i].range         = value.mSize;
          writeInfos[i].descriptorType = vk::DescriptorType::eStorageBuffer;
          writeInfos[i].pBufferInfo    = &bufferInfos[i];

        } else if (std::holds_alternative<DynamicStorageBufferBinding>(binding.second)) {

          auto value                   = std::get<DynamicStorageBufferBinding>(binding.second);
          bufferInfos[i].buffer        = *value.mBuffer->mBuffer;
          bufferInfos[i].range         = value.mSize;
          writeInfos[i].descriptorType = vk::DescriptorType::eStorageBufferDynamic;
          writeInfos[i].pBufferInfo    = &bufferInfos[i];

          dynamicOffsets.push_back(mBindingState.getDynamicOffset(setNum, binding.first));
        }

        ++i;
      }

      // Do the actual update of the DescriptorSet.
      if (bindingCount > 0) {
        mDevice->getHandle()->updateDescriptorSets(writeInfos, nullptr);
      }

      // Now the DescriptorSet is up-to-date and we can bind it.
      mVkCmd->bindDescriptorSets(bindPoint, *mCurrentShader->getReflection()->getLayout(), setNum,
          *descriptorSet, dynamicOffsets);

      // Store the hash of the DescriptorSet layout so that we can check for
      // compatibility if a new program is bound.
      mCurrentDescriptorSets[setNum] = {descriptorSet, setReflections[setNum]->getHash()};

    }
    // There is a matching DescriptorSet currently bound,
    // however the dynamic offsets have been changed.
    else if (mBindingState.getDirtyDynamicOffsets().find(setNum) !=
             mBindingState.getDirtyDynamicOffsets().end()) {

      std::vector<uint32_t> dynamicOffsets;

      for (auto const& binding : mBindingState.getBindings(setNum)) {
        if (std::holds_alternative<DynamicUniformBufferBinding>(binding.second)) {
          dynamicOffsets.push_back(mBindingState.getDynamicOffset(setNum, binding.first));
        }
      }

      mVkCmd->bindDescriptorSets(bindPoint, *mCurrentShader->getReflection()->getLayout(), setNum,
          *currentSetIt->second.mSet, dynamicOffsets);
    }
  }

  // Reset dirty state.
  mBindingState.clearDirtySets();
  mBindingState.clearDirtyDynamicOffsets();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelinePtr CommandBuffer::getPipelineHandle() {

  if (!mCurrentShader) {
    throw std::runtime_error("Failed to create pipeline for CommandBuffer \"" + getName() +
                             "\": There must be an active Shader!");
  }

  if (mType == QueueType::eCompute) {

    Core::BitHash hash;
    hash.push<64>(mCurrentShader.get());

    auto const& specialisationHash = mSpecialisationState.getHash();
    hash.insert(hash.end(), specialisationHash.begin(), specialisationHash.end());

    auto cached = mPipelineCache.find(hash);
    if (cached != mPipelineCache.end()) {
      cached->second.second = mRecordingID;
      return cached->second.first;
    }

    vk::ComputePipelineCreateInfo info;

    if (mCurrentShader->getModules().size() != 1) {
      throw std::runtime_error("Failed to create compute pipeline for CommandBuffer \"" +
                               getName() + "\": There must be exactly one ShaderModule!");
    }

    info.stage.stage               = mCurrentShader->getModules()[0]->getStage();
    info.stage.module              = *mCurrentShader->getModules()[0]->getHandle();
    info.stage.pName               = "main";
    info.stage.pSpecializationInfo = mSpecialisationState.getInfo();
    info.layout                    = *mCurrentShader->getReflection()->getLayout();

    auto pipeline = mDevice->createComputePipeline("ComputePipeline of " + getName(), info);

    mPipelineCache[hash] = {pipeline, mRecordingID};

    return pipeline;
  }

  // -----------------------------------------------------------------------------------------------

  Core::BitHash hash = mGraphicsState.getHash();

  for (auto const& m : mCurrentShader->getModules()) {
    hash.push<64>(m->getHandle().get());
  }
  hash.push<64>(mCurrentRenderPass.get());
  hash.push<32>(mCurrentSubpass);

  auto const& specialisationHash = mSpecialisationState.getHash();
  hash.insert(hash.end(), specialisationHash.begin(), specialisationHash.end());

  auto cached = mPipelineCache.find(hash);
  if (cached != mPipelineCache.end()) {
    cached->second.second = mRecordingID;
    return cached->second.first;
  }

  // -----------------------------------------------------------------------------------------------
  std::vector<vk::PipelineShaderStageCreateInfo> stageInfos;
  if (mCurrentShader) {
    for (auto const& i : mCurrentShader->getModules()) {
      vk::PipelineShaderStageCreateInfo stageInfo;
      stageInfo.stage               = i->getStage();
      stageInfo.module              = *i->getHandle();
      stageInfo.pName               = "main";
      stageInfo.pSpecializationInfo = mSpecialisationState.getInfo();
      stageInfos.push_back(stageInfo);
    }
  }

  // -----------------------------------------------------------------------------------------------
  vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;

  std::vector<vk::VertexInputBindingDescription>   vertexInputBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
  for (auto const& i : mGraphicsState.getVertexInputBindings()) {
    vertexInputBindingDescriptions.emplace_back(i.binding, i.stride, i.inputRate);
  }
  for (auto const& i : mGraphicsState.getVertexInputAttributes()) {
    vertexInputAttributeDescriptions.emplace_back(i.location, i.binding, i.format, i.offset);
  }
  vertexInputStateInfo.vertexBindingDescriptionCount =
      static_cast<uint32_t>(vertexInputBindingDescriptions.size());
  vertexInputStateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
  vertexInputStateInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
  vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
  inputAssemblyStateInfo.topology = mGraphicsState.getTopology();
  inputAssemblyStateInfo.primitiveRestartEnable =
      static_cast<vk::Bool32>(mGraphicsState.getPrimitiveRestartEnable());

  // -----------------------------------------------------------------------------------------------
  vk::PipelineTessellationStateCreateInfo tessellationStateInfo;
  tessellationStateInfo.patchControlPoints = mGraphicsState.getTessellationPatchControlPoints();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineViewportStateCreateInfo viewportStateInfo;
  std::vector<vk::Viewport>           viewports;
  std::vector<vk::Rect2D>             scissors;
  for (auto const& i : mGraphicsState.getViewports()) {
    viewports.emplace_back(
        i.mOffset[0], i.mOffset[1], i.mExtend[0], i.mExtend[1], i.mMinDepth, i.mMaxDepth);
  }

  // use viewport as scissors if no scissors are defined
  if (!mGraphicsState.getScissors().empty()) {
    for (auto const& i : mGraphicsState.getScissors()) {
      scissors.push_back({{i.mOffset[0], i.mOffset[1]}, {i.mExtend[0], i.mExtend[1]}});
    }
  } else {
    for (auto const& i : mGraphicsState.getViewports()) {
      scissors.push_back({{static_cast<int32_t>(i.mOffset[0]), static_cast<int32_t>(i.mOffset[1])},
          {static_cast<uint32_t>(i.mExtend[0]), static_cast<uint32_t>(i.mExtend[1])}});
    }
  }
  viewportStateInfo.viewportCount = static_cast<uint32_t>(viewports.size());
  viewportStateInfo.pViewports    = viewports.data();
  viewportStateInfo.scissorCount  = static_cast<uint32_t>(scissors.size());
  viewportStateInfo.pScissors     = scissors.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;
  rasterizationStateInfo.depthClampEnable =
      static_cast<vk::Bool32>(mGraphicsState.getDepthClampEnable());
  rasterizationStateInfo.rasterizerDiscardEnable =
      static_cast<vk::Bool32>(mGraphicsState.getRasterizerDiscardEnable());
  rasterizationStateInfo.polygonMode = mGraphicsState.getPolygonMode();
  rasterizationStateInfo.cullMode    = mGraphicsState.getCullMode();
  rasterizationStateInfo.frontFace   = mGraphicsState.getFrontFace();
  rasterizationStateInfo.depthBiasEnable =
      static_cast<vk::Bool32>(mGraphicsState.getDepthBiasEnable());
  rasterizationStateInfo.depthBiasConstantFactor = mGraphicsState.getDepthBiasConstantFactor();
  rasterizationStateInfo.depthBiasClamp          = mGraphicsState.getDepthBiasClamp();
  rasterizationStateInfo.depthBiasSlopeFactor    = mGraphicsState.getDepthBiasSlopeFactor();
  rasterizationStateInfo.lineWidth               = mGraphicsState.getLineWidth();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
  multisampleStateInfo.rasterizationSamples = mGraphicsState.getRasterizationSamples();
  multisampleStateInfo.sampleShadingEnable =
      static_cast<vk::Bool32>(mGraphicsState.getSampleShadingEnable());
  multisampleStateInfo.minSampleShading = mGraphicsState.getMinSampleShading();
  multisampleStateInfo.pSampleMask      = mGraphicsState.getSampleMask().data();
  multisampleStateInfo.alphaToCoverageEnable =
      static_cast<vk::Bool32>(mGraphicsState.getAlphaToCoverageEnable());
  multisampleStateInfo.alphaToOneEnable =
      static_cast<vk::Bool32>(mGraphicsState.getAlphaToOneEnable());

  // -----------------------------------------------------------------------------------------------
  vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo;
  depthStencilStateInfo.depthTestEnable =
      static_cast<vk::Bool32>(mGraphicsState.getDepthTestEnable());
  depthStencilStateInfo.depthWriteEnable =
      static_cast<vk::Bool32>(mGraphicsState.getDepthWriteEnable());
  depthStencilStateInfo.depthCompareOp = mGraphicsState.getDepthCompareOp();
  depthStencilStateInfo.depthBoundsTestEnable =
      static_cast<vk::Bool32>(mGraphicsState.getDepthBoundsTestEnable());
  depthStencilStateInfo.stencilTestEnable =
      static_cast<vk::Bool32>(mGraphicsState.getStencilTestEnable());
  depthStencilStateInfo.front          = {mGraphicsState.getStencilFrontFailOp(),
      mGraphicsState.getStencilFrontPassOp(), mGraphicsState.getStencilFrontDepthFailOp(),
      mGraphicsState.getStencilFrontCompareOp(), mGraphicsState.getStencilFrontCompareMask(),
      mGraphicsState.getStencilFrontWriteMask(), mGraphicsState.getStencilFrontReference()};
  depthStencilStateInfo.back           = {mGraphicsState.getStencilBackFailOp(),
      mGraphicsState.getStencilBackPassOp(), mGraphicsState.getStencilBackDepthFailOp(),
      mGraphicsState.getStencilBackCompareOp(), mGraphicsState.getStencilBackCompareMask(),
      mGraphicsState.getStencilBackWriteMask(), mGraphicsState.getStencilBackReference()};
  depthStencilStateInfo.minDepthBounds = mGraphicsState.getMinDepthBounds();
  depthStencilStateInfo.maxDepthBounds = mGraphicsState.getMaxDepthBounds();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineColorBlendStateCreateInfo              colorBlendStateInfo;
  std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachments;

  // use default blend attachments if none are defined
  if (mGraphicsState.getBlendAttachments().empty()) {
    auto attachmentCount =
        int32_t(mCurrentRenderPass->getSubpasses()[mCurrentSubpass].mColorAttachments.size());

    for (int32_t i(0); i < attachmentCount; ++i) {
      GraphicsState::BlendAttachment a;
      pipelineColorBlendAttachments.emplace_back(static_cast<vk::Bool32>(a.mBlendEnable),
          a.mSrcColorBlendFactor, a.mDstColorBlendFactor, a.mColorBlendOp, a.mSrcAlphaBlendFactor,
          a.mDstAlphaBlendFactor, a.mAlphaBlendOp, a.mColorWriteMask);
    }

  } else {
    for (auto const& i : mGraphicsState.getBlendAttachments()) {
      pipelineColorBlendAttachments.emplace_back(static_cast<vk::Bool32>(i.mBlendEnable),
          i.mSrcColorBlendFactor, i.mDstColorBlendFactor, i.mColorBlendOp, i.mSrcAlphaBlendFactor,
          i.mDstAlphaBlendFactor, i.mAlphaBlendOp, i.mColorWriteMask);
    }
  }
  colorBlendStateInfo.logicOpEnable =
      static_cast<vk::Bool32>(mGraphicsState.getBlendLogicOpEnable());
  colorBlendStateInfo.logicOp         = mGraphicsState.getBlendLogicOp();
  colorBlendStateInfo.attachmentCount = static_cast<uint32_t>(pipelineColorBlendAttachments.size());
  colorBlendStateInfo.pAttachments    = pipelineColorBlendAttachments.data();
  colorBlendStateInfo.blendConstants[0] = mGraphicsState.getBlendConstants()[0];
  colorBlendStateInfo.blendConstants[1] = mGraphicsState.getBlendConstants()[1];
  colorBlendStateInfo.blendConstants[2] = mGraphicsState.getBlendConstants()[2];
  colorBlendStateInfo.blendConstants[3] = mGraphicsState.getBlendConstants()[3];

  // -----------------------------------------------------------------------------------------------
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  std::vector<vk::DynamicState>      dynamicState(
      mGraphicsState.getDynamicState().begin(), mGraphicsState.getDynamicState().end());
  dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicState.size());
  dynamicStateInfo.pDynamicStates    = dynamicState.data();

  // -----------------------------------------------------------------------------------------------
  vk::GraphicsPipelineCreateInfo info;
  info.stageCount          = static_cast<uint32_t>(stageInfos.size());
  info.pStages             = stageInfos.data();
  info.pVertexInputState   = &vertexInputStateInfo;
  info.pInputAssemblyState = &inputAssemblyStateInfo;
  info.pTessellationState  = &tessellationStateInfo;
  info.pViewportState      = &viewportStateInfo;
  info.pRasterizationState = &rasterizationStateInfo;
  info.pMultisampleState   = &multisampleStateInfo;
  info.pDepthStencilState  = &depthStencilStateInfo;
  info.pColorBlendState    = &colorBlendStateInfo;
  if (!mGraphicsState.getDynamicState().empty()) {
    info.pDynamicState = &dynamicStateInfo;
  }
  info.renderPass = *mCurrentRenderPass->getHandle();
  info.subpass    = mCurrentSubpass;

  if (mCurrentShader) {
    info.layout = *mCurrentShader->getReflection()->getLayout();
  }

  auto pipeline = mDevice->createGraphicsPipeline("GraphicsPipeline of " + getName(), info);

  mPipelineCache[hash] = {pipeline, mRecordingID};

  return pipeline;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
