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
#include "Device.hpp"
#include "PipelineReflection.hpp"
#include "RenderPass.hpp"
#include "ShaderModule.hpp"
#include "ShaderProgram.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

CommandBuffer::CommandBuffer(DevicePtr const& device, QueueType type, vk::CommandBufferLevel level)
  : mDevice(device)
  , mVkCmd(device->allocateCommandBuffer(type, level))
  , mType(type)
  , mLevel(level)
  , mGraphicsState(device)
  , mDescriptorSetCache(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::reset() {
  mCurrentDescriptorSetLayoutHashes.clear();
  mDescriptorSetCache.releaseAll();
  mVkCmd->reset({});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::begin(vk::CommandBufferUsageFlagBits usage) const { mVkCmd->begin({usage}); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::end() const { mVkCmd->end(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::submit(std::vector<vk::Semaphore> const& waitSemaphores,
  std::vector<vk::PipelineStageFlags> const&                 waitStages,
  std::vector<vk::Semaphore> const& signalSemaphores, vk::Fence const& fence) const {

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

void CommandBuffer::beginRenderPass(RenderPassPtr const& renderPass) {
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
  mCurrentSubPass    = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::endRenderPass() {
  mVkCmd->endRenderPass();
  mCurrentRenderPass.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindIndexBuffer(
  BackedBufferPtr const& buffer, vk::DeviceSize offset, vk::IndexType indexType) const {
  mVkCmd->bindIndexBuffer(*buffer->mBuffer, offset, indexType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::bindVertexBuffers(uint32_t                   firstBinding,
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

void CommandBuffer::bindCombinedImageSampler(
  TexturePtr const& texture, uint32_t set, uint32_t binding) const {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::draw(
  uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {

  flush();
  mVkCmd->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
  int32_t vertexOffset, uint32_t firstInstance) {

  flush();
  mVkCmd->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
  flush();
  mVkCmd->dispatch(groupCountX, groupCountY, groupCountZ);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::setShaderProgram(ShaderProgramPtr const& val) { mCurrentShaderProgram = val; }

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgramPtr const& CommandBuffer::getShaderProgram() const { return mCurrentShaderProgram; }

////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsState& CommandBuffer::graphicsState() { return mGraphicsState; }

////////////////////////////////////////////////////////////////////////////////////////////////////

BindingState& CommandBuffer::bindingState() { return mBindingState; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::pushConstants(const void* data, uint32_t size, uint32_t offset) const {
  auto const& reflection = mCurrentShaderProgram->getReflection();
  auto constants = reflection->getResources(PipelineResource::ResourceType::ePushConstantBuffer);

  if (constants.size() != 1) {
    throw std::runtime_error("Failed to set push constants: There must be exactly one "
                             "PushConstantBuffer defined in the pipeline reflection!");
  }

  mVkCmd->pushConstants(
    *reflection->getLayout(), constants.begin()->second.mStages, offset, size, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout,
  vk::ImageLayout newLayout, vk::PipelineStageFlagBits srcStage, vk::PipelineStageFlagBits dstStage,
  vk::ImageSubresourceRange range) const {

  // clang-format off
  static const std::unordered_map<vk::ImageLayout, vk::AccessFlags> accessMapping = {
    {vk::ImageLayout::eUndefined,                     vk::AccessFlags()},
    {vk::ImageLayout::eGeneral,                       vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite},
    {vk::ImageLayout::eColorAttachmentOptimal,        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite},
    {vk::ImageLayout::eDepthStencilReadOnlyOptimal,   vk::AccessFlagBits::eDepthStencilAttachmentRead},
    {vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite},
    {vk::ImageLayout::eShaderReadOnlyOptimal,         vk::AccessFlagBits::eShaderRead},
    {vk::ImageLayout::eTransferDstOptimal,            vk::AccessFlagBits::eTransferWrite},
    {vk::ImageLayout::eTransferSrcOptimal,            vk::AccessFlagBits::eTransferRead},
    {vk::ImageLayout::ePresentSrcKHR,                 vk::AccessFlagBits::eMemoryRead}
  };
  // clang-format on

  auto srcAccess = accessMapping.find(oldLayout);
  auto dstAccess = accessMapping.find(newLayout);

  if (srcAccess == accessMapping.end() || dstAccess == accessMapping.end()) {
    throw std::runtime_error("Failed to transition image layout: Unsupported transition!");
  }

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout           = oldLayout;
  barrier.newLayout           = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image               = image;
  barrier.subresourceRange    = range;
  barrier.srcAccessMask       = srcAccess->second;
  barrier.dstAccessMask       = dstAccess->second;

  mVkCmd->pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits(), nullptr, nullptr, barrier);
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
  vk::ImageLayout dstLayout, vk::ImageResolve region) const {
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

void CommandBuffer::flush() {

  vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;

  if (mType == QueueType::eCompute) {
    bindPoint = vk::PipelineBindPoint::eCompute;
  }

  auto pipeline = getPipelineHandle();
  mVkCmd->bindPipeline(bindPoint, *pipeline);

  // for each set of current program
  //    if binding state is dirty
  //    or no currently bound set
  //    or currently bound layout does not match layout of set
  //         acquire new set
  //         update descriptor set
  //         bind descriptor set
  //         store it

  for (auto setReflection : mCurrentShaderProgram->getDescriptorSetReflections()) {

    uint32_t setNum = setReflection.first;

    auto currentHashIt = mCurrentDescriptorSetLayoutHashes.find(setNum);

    if (mBindingState.getDirtySets().find(setNum) != mBindingState.getDirtySets().end() ||
        currentHashIt == mCurrentDescriptorSetLayoutHashes.end() ||
        currentHashIt->second != setReflection.second->getHash()) {

      auto descriptorSet = mDescriptorSetCache.acquireHandle(
        mCurrentShaderProgram->getDescriptorSetReflections().at(setNum));

      for (auto const& binding : mBindingState.getBindings(setNum)) {

        if (std::holds_alternative<CombinedImageSamplerBinding>(binding.second)) {

          auto                    value = std::get<CombinedImageSamplerBinding>(binding.second);
          vk::DescriptorImageInfo imageInfo;
          imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
          imageInfo.imageView   = *value.mTexture->mBackedImage->mView;
          imageInfo.sampler     = *value.mTexture->mSampler;

          vk::WriteDescriptorSet info;
          info.dstSet          = *descriptorSet;
          info.dstBinding      = binding.first;
          info.dstArrayElement = 0;
          info.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
          info.descriptorCount = 1;
          info.pImageInfo      = &imageInfo;

          mDevice->getHandle()->updateDescriptorSets(info, nullptr);

        } else if (std::holds_alternative<StorageImageBinding>(binding.second)) {

          auto                    value = std::get<StorageImageBinding>(binding.second);
          vk::DescriptorImageInfo imageInfo;
          imageInfo.imageLayout = vk::ImageLayout::eGeneral;
          imageInfo.imageView   = *value.mImage->mBackedImage->mView;
          imageInfo.sampler     = *value.mImage->mSampler;

          vk::WriteDescriptorSet info;
          info.dstSet          = *descriptorSet;
          info.dstBinding      = binding.first;
          info.dstArrayElement = 0;
          info.descriptorType  = vk::DescriptorType::eStorageImage;
          info.descriptorCount = 1;
          info.pImageInfo      = &imageInfo;

          mDevice->getHandle()->updateDescriptorSets(info, nullptr);

        } else if (std::holds_alternative<UniformBufferBinding>(binding.second)) {

          auto                     value = std::get<UniformBufferBinding>(binding.second);
          vk::DescriptorBufferInfo bufferInfo;
          bufferInfo.buffer = *value.mBuffer->mBuffer;
          bufferInfo.offset = value.mOffset;
          bufferInfo.range  = value.mSize;

          vk::WriteDescriptorSet info;
          info.dstSet          = *descriptorSet;
          info.dstBinding      = binding.first;
          info.dstArrayElement = 0;
          info.descriptorType  = vk::DescriptorType::eUniformBuffer;
          info.descriptorCount = 1;
          info.pBufferInfo     = &bufferInfo;

          mDevice->getHandle()->updateDescriptorSets(info, nullptr);
        }
      }

      mVkCmd->bindDescriptorSets(bindPoint, *mCurrentShaderProgram->getReflection()->getLayout(),
        setNum, *descriptorSet, {});

      mCurrentDescriptorSetLayoutHashes[setNum] = setReflection.second->getHash();
    }
  }

  mBindingState.clearDirtySets();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelinePtr CommandBuffer::getPipelineHandle() {

  if (mType == QueueType::eCompute) {

    if (!mCurrentShaderProgram) {
      throw std::runtime_error("Failed to create compute pipeline: No ShaderProgram given!");
    }

    Core::BitHash hash;
    hash.push<64>(mCurrentShaderProgram.get());

    auto cached = mPipelineCache.find(hash);
    if (cached != mPipelineCache.end()) {
      return cached->second;
    }

    vk::ComputePipelineCreateInfo info;

    if (mCurrentShaderProgram->getModules().size() != 1) {
      throw std::runtime_error(
        "Failed to create compute pipeline: There must be exactly one ShaderModule!");
    }

    info.stage.stage               = mCurrentShaderProgram->getModules()[0]->getStage();
    info.stage.module              = *mCurrentShaderProgram->getModules()[0]->getModule();
    info.stage.pName               = "main";
    info.stage.pSpecializationInfo = nullptr;
    info.layout                    = *mCurrentShaderProgram->getReflection()->getLayout();

    auto pipeline = mDevice->createComputePipeline(info);

    mPipelineCache[hash] = pipeline;

    return pipeline;
  }

  // -----------------------------------------------------------------------------------------------

  Core::BitHash hash = mGraphicsState.getHash();
  hash.push<64>(mCurrentShaderProgram.get());
  hash.push<64>(mCurrentRenderPass.get());
  hash.push<32>(mCurrentSubPass);

  auto cached = mPipelineCache.find(hash);
  if (cached != mPipelineCache.end()) {
    return cached->second;
  }

  // -----------------------------------------------------------------------------------------------
  std::vector<vk::PipelineShaderStageCreateInfo> stageInfos;
  if (mCurrentShaderProgram) {
    for (auto const& i : mCurrentShaderProgram->getModules()) {
      vk::PipelineShaderStageCreateInfo stageInfo;
      stageInfo.stage               = i->getStage();
      stageInfo.module              = *i->getModule();
      stageInfo.pName               = "main";
      stageInfo.pSpecializationInfo = nullptr;
      stageInfos.push_back(stageInfo);
    }
  }

  // -----------------------------------------------------------------------------------------------
  vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;

  std::vector<vk::VertexInputBindingDescription>   vertexInputBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
  for (auto const& i : mGraphicsState.getVertexInputBindings()) {
    vertexInputBindingDescriptions.push_back({i.binding, i.stride, i.inputRate});
  }
  for (auto const& i : mGraphicsState.getVertexInputAttributes()) {
    vertexInputAttributeDescriptions.push_back({i.location, i.binding, i.format, i.offset});
  }
  vertexInputStateInfo.vertexBindingDescriptionCount   = vertexInputBindingDescriptions.size();
  vertexInputStateInfo.pVertexBindingDescriptions      = vertexInputBindingDescriptions.data();
  vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
  vertexInputStateInfo.pVertexAttributeDescriptions    = vertexInputAttributeDescriptions.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
  inputAssemblyStateInfo.topology               = mGraphicsState.getTopology();
  inputAssemblyStateInfo.primitiveRestartEnable = mGraphicsState.getPrimitiveRestartEnable();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineTessellationStateCreateInfo tessellationStateInfo;
  tessellationStateInfo.patchControlPoints = mGraphicsState.getTessellationPatchControlPoints();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineViewportStateCreateInfo viewportStateInfo;
  std::vector<vk::Viewport>           viewports;
  std::vector<vk::Rect2D>             scissors;
  for (auto const& i : mGraphicsState.getViewports()) {
    viewports.push_back(
      {i.mOffset[0], i.mOffset[1], i.mExtend[0], i.mExtend[1], i.mMinDepth, i.mMaxDepth});
  }

  // use viewport as scissors if no scissors are defined
  if (mGraphicsState.getScissors().size() > 0) {
    for (auto const& i : mGraphicsState.getScissors()) {
      scissors.push_back({{i.mOffset[0], i.mOffset[1]}, {i.mExtend[0], i.mExtend[1]}});
    }
  } else {
    for (auto const& i : mGraphicsState.getViewports()) {
      scissors.push_back({{(int32_t)i.mOffset[0], (int32_t)i.mOffset[1]},
        {(uint32_t)i.mExtend[0], (uint32_t)i.mExtend[1]}});
    }
  }
  viewportStateInfo.viewportCount = viewports.size();
  viewportStateInfo.pViewports    = viewports.data();
  viewportStateInfo.scissorCount  = scissors.size();
  viewportStateInfo.pScissors     = scissors.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;
  rasterizationStateInfo.depthClampEnable        = mGraphicsState.getDepthClampEnable();
  rasterizationStateInfo.rasterizerDiscardEnable = mGraphicsState.getRasterizerDiscardEnable();
  rasterizationStateInfo.polygonMode             = mGraphicsState.getPolygonMode();
  rasterizationStateInfo.cullMode                = mGraphicsState.getCullMode();
  rasterizationStateInfo.frontFace               = mGraphicsState.getFrontFace();
  rasterizationStateInfo.depthBiasEnable         = mGraphicsState.getDepthBiasEnable();
  rasterizationStateInfo.depthBiasConstantFactor = mGraphicsState.getDepthBiasConstantFactor();
  rasterizationStateInfo.depthBiasClamp          = mGraphicsState.getDepthBiasClamp();
  rasterizationStateInfo.depthBiasSlopeFactor    = mGraphicsState.getDepthBiasSlopeFactor();
  rasterizationStateInfo.lineWidth               = mGraphicsState.getLineWidth();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
  multisampleStateInfo.rasterizationSamples  = mGraphicsState.getRasterizationSamples();
  multisampleStateInfo.sampleShadingEnable   = mGraphicsState.getSampleShadingEnable();
  multisampleStateInfo.minSampleShading      = mGraphicsState.getMinSampleShading();
  multisampleStateInfo.pSampleMask           = mGraphicsState.getSampleMask().data();
  multisampleStateInfo.alphaToCoverageEnable = mGraphicsState.getAlphaToCoverageEnable();
  multisampleStateInfo.alphaToOneEnable      = mGraphicsState.getAlphaToOneEnable();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo;
  depthStencilStateInfo.depthTestEnable       = mGraphicsState.getDepthTestEnable();
  depthStencilStateInfo.depthWriteEnable      = mGraphicsState.getDepthWriteEnable();
  depthStencilStateInfo.depthCompareOp        = mGraphicsState.getDepthCompareOp();
  depthStencilStateInfo.depthBoundsTestEnable = mGraphicsState.getDepthBoundsTestEnable();
  depthStencilStateInfo.stencilTestEnable     = mGraphicsState.getStencilTestEnable();
  depthStencilStateInfo.front                 = {mGraphicsState.getStencilFrontFailOp(),
    mGraphicsState.getStencilFrontPassOp(), mGraphicsState.getStencilFrontDepthFailOp(),
    mGraphicsState.getStencilFrontCompareOp(), mGraphicsState.getStencilFrontCompareMask(),
    mGraphicsState.getStencilFrontWriteMask(), mGraphicsState.getStencilFrontReference()};
  depthStencilStateInfo.back                  = {mGraphicsState.getStencilBackFailOp(),
    mGraphicsState.getStencilBackPassOp(), mGraphicsState.getStencilBackDepthFailOp(),
    mGraphicsState.getStencilBackCompareOp(), mGraphicsState.getStencilBackCompareMask(),
    mGraphicsState.getStencilBackWriteMask(), mGraphicsState.getStencilBackReference()};
  depthStencilStateInfo.minDepthBounds        = mGraphicsState.getMinDepthBounds();
  depthStencilStateInfo.maxDepthBounds        = mGraphicsState.getMaxDepthBounds();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineColorBlendStateCreateInfo              colorBlendStateInfo;
  std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachments;
  for (auto const& i : mGraphicsState.getBlendAttachments()) {
    pipelineColorBlendAttachments.push_back(
      {i.mBlendEnable, i.mSrcColorBlendFactor, i.mDstColorBlendFactor, i.mColorBlendOp,
        i.mSrcAlphaBlendFactor, i.mDstAlphaBlendFactor, i.mAlphaBlendOp, i.mColorWriteMask});
  }
  colorBlendStateInfo.logicOpEnable     = mGraphicsState.getBlendLogicOpEnable();
  colorBlendStateInfo.logicOp           = mGraphicsState.getBlendLogicOp();
  colorBlendStateInfo.attachmentCount   = pipelineColorBlendAttachments.size();
  colorBlendStateInfo.pAttachments      = pipelineColorBlendAttachments.data();
  colorBlendStateInfo.blendConstants[0] = mGraphicsState.getBlendConstants()[0];
  colorBlendStateInfo.blendConstants[1] = mGraphicsState.getBlendConstants()[1];
  colorBlendStateInfo.blendConstants[2] = mGraphicsState.getBlendConstants()[2];
  colorBlendStateInfo.blendConstants[3] = mGraphicsState.getBlendConstants()[3];

  // -----------------------------------------------------------------------------------------------
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  std::vector<vk::DynamicState>      dynamicState(
    mGraphicsState.getDynamicState().begin(), mGraphicsState.getDynamicState().end());
  dynamicStateInfo.dynamicStateCount = dynamicState.size();
  dynamicStateInfo.pDynamicStates    = dynamicState.data();

  // -----------------------------------------------------------------------------------------------
  vk::GraphicsPipelineCreateInfo info;
  info.stageCount          = stageInfos.size();
  info.pStages             = stageInfos.data();
  info.pVertexInputState   = &vertexInputStateInfo;
  info.pInputAssemblyState = &inputAssemblyStateInfo;
  info.pTessellationState  = &tessellationStateInfo;
  info.pViewportState      = &viewportStateInfo;
  info.pRasterizationState = &rasterizationStateInfo;
  info.pMultisampleState   = &multisampleStateInfo;
  info.pDepthStencilState  = &depthStencilStateInfo;
  info.pColorBlendState    = &colorBlendStateInfo;
  if (mGraphicsState.getDynamicState().size() > 0) {
    info.pDynamicState = &dynamicStateInfo;
  }
  info.renderPass = *mCurrentRenderPass->getHandle();
  info.subpass    = mCurrentSubPass;

  if (mCurrentShaderProgram) {
    info.layout = *mCurrentShaderProgram->getReflection()->getLayout();
  }

  auto pipeline = mDevice->createGraphicsPipeline(info);

  mPipelineCache[hash] = pipeline;

  return pipeline;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
