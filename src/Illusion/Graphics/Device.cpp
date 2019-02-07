////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Device.hpp"

#include "../Core/EnumCast.hpp"
#include "../Core/Logger.hpp"
#include "BackedBuffer.hpp"
#include "BackedImage.hpp"
#include "CommandBuffer.hpp"
#include "PhysicalDevice.hpp"
#include "PipelineResource.hpp"
#include "Texture.hpp"
#include "Utils.hpp"
#include "VulkanPtr.hpp"

#include <iostream>
#include <set>
#include <utility>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
const std::vector<const char*> DEVICE_EXTENSIONS{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Device::Device(std::string const& name, PhysicalDevicePtr physicalDevice)
    : Core::NamedObject(name)
    , mPhysicalDevice(std::move(physicalDevice))
    , mDevice(createDevice(name)) {

  mSetObjectNameFunc =
      PFN_vkSetDebugUtilsObjectNameEXT(mDevice->getProcAddr("vkSetDebugUtilsObjectNameEXT"));

  for (size_t i(0); i < 3; ++i) {
    auto type  = static_cast<QueueType>(i);
    mQueues[i] = mDevice->getQueue(mPhysicalDevice->getQueueFamily(type), 0);

    vk::CommandPoolCreateInfo info;
    info.queueFamilyIndex = mPhysicalDevice->getQueueFamily(type);
    info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    mCommandPools[i] =
        createCommandPool("CommandPool for QueueFamilyIndex " +
                              std::to_string(info.queueFamilyIndex) + " of " + getName(),
            info);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Device::~Device() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedImagePtr Device::createBackedImage(std::string const& name, vk::ImageCreateInfo imageInfo,
    vk::ImageViewType viewType, const vk::ImageAspectFlags& imageAspectMask,
    const vk::MemoryPropertyFlags& properties, vk::ImageLayout layout,
    vk::ComponentMapping const& componentMapping, vk::DeviceSize dataSize, const void* data) const {

  auto result   = std::make_shared<BackedImage>();
  result->mName = name;

  // make sure eTransferDst is set when we have data to upload
  if (data != nullptr) {
    imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
  }

  result->mImageInfo     = imageInfo;
  result->mImage         = createImage(name, imageInfo);
  result->mCurrentLayout = imageInfo.initialLayout;

  // create memory
  auto requirements = mDevice->getImageMemoryRequirements(*result->mImage);

  result->mMemoryInfo.allocationSize = requirements.size;
  result->mMemoryInfo.memoryTypeIndex =
      mPhysicalDevice->findMemoryType(requirements.memoryTypeBits, properties);
  result->mMemory = createMemory("Memory for " + name, result->mMemoryInfo);
  mDevice->bindImageMemory(*result->mImage, *result->mMemory, 0);

  // create image view
  result->mViewInfo.image                           = *result->mImage;
  result->mViewInfo.viewType                        = viewType;
  result->mViewInfo.format                          = imageInfo.format;
  result->mViewInfo.subresourceRange.aspectMask     = imageAspectMask;
  result->mViewInfo.subresourceRange.baseMipLevel   = 0;
  result->mViewInfo.subresourceRange.levelCount     = imageInfo.mipLevels;
  result->mViewInfo.subresourceRange.baseArrayLayer = 0;
  result->mViewInfo.subresourceRange.layerCount     = imageInfo.arrayLayers;
  result->mViewInfo.components                      = componentMapping;

  result->mView = createImageView("ImageView for " + name, result->mViewInfo);

  if (data != nullptr) {
    auto cmd = allocateCommandBuffer("Upload to BackedImage");
    cmd->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    auto stagingBuffer =
        createBackedBuffer("StagingBuffer for " + name, vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            dataSize, data);

    vk::ImageMemoryBarrier barrier;
    barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                       = *result->mImage;
    barrier.subresourceRange.levelCount = imageInfo.mipLevels;
    barrier.subresourceRange.layerCount = imageInfo.arrayLayers;
    barrier.subresourceRange.aspectMask = imageAspectMask;
    barrier.oldLayout                   = result->mCurrentLayout;
    barrier.newLayout                   = vk::ImageLayout::eTransferDstOptimal;

    cmd->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlagBits(), nullptr, nullptr, barrier);

    std::vector<vk::BufferImageCopy> infos;
    uint64_t                         offset    = 0;
    uint32_t                         mipWidth  = imageInfo.extent.width;
    uint32_t                         mipHeight = imageInfo.extent.height;

    for (uint32_t i = 0; i < imageInfo.mipLevels; ++i) {
      uint64_t size = mipWidth * mipHeight * Utils::getByteCount(imageInfo.format);

      if (offset + size > dataSize) {
        break;
      }

      vk::BufferImageCopy info;
      info.imageSubresource.aspectMask     = imageAspectMask;
      info.imageSubresource.mipLevel       = i;
      info.imageSubresource.baseArrayLayer = 0;
      info.imageSubresource.layerCount     = imageInfo.arrayLayers;
      info.imageExtent.width               = mipWidth;
      info.imageExtent.height              = mipHeight;
      info.imageExtent.depth               = 1;
      info.bufferOffset                    = offset;

      infos.push_back(info);

      offset += size;
      mipWidth  = std::max(mipWidth / 2, 1u);
      mipHeight = std::max(mipHeight / 2, 1u);
    }

    cmd->copyBufferToImage(
        *stagingBuffer->mBuffer, *result->mImage, vk::ImageLayout::eTransferDstOptimal, infos);

    result->mCurrentLayout = vk::ImageLayout::eTransferDstOptimal;

    if (layout != vk::ImageLayout::eUndefined) {
      barrier.oldLayout = result->mCurrentLayout;
      barrier.newLayout = layout;

      cmd->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
          vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits(), nullptr, nullptr,
          barrier);

      result->mCurrentLayout = layout;
    }

    cmd->end();

    vk::CommandBuffer bufs[] = {*cmd};
    vk::SubmitInfo    info;
    info.commandBufferCount = 1;
    info.pCommandBuffers    = bufs;

    getQueue(QueueType::eGeneric).submit(info, nullptr);
    getQueue(QueueType::eGeneric).waitIdle();

  } else if (layout != vk::ImageLayout::eUndefined) {

    auto cmd = allocateCommandBuffer("Transition image layout");
    cmd->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    vk::ImageMemoryBarrier barrier;
    barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                       = *result->mImage;
    barrier.subresourceRange.levelCount = imageInfo.mipLevels;
    barrier.subresourceRange.layerCount = imageInfo.arrayLayers;
    barrier.subresourceRange.aspectMask = imageAspectMask;
    barrier.oldLayout                   = result->mCurrentLayout;
    barrier.newLayout                   = layout;

    cmd->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlagBits(), nullptr, nullptr, barrier);

    result->mCurrentLayout = layout;

    cmd->end();

    vk::CommandBuffer bufs[] = {*cmd};
    vk::SubmitInfo    info;
    info.commandBufferCount = 1;
    info.pCommandBuffers    = bufs;

    getQueue(QueueType::eGeneric).submit(info, nullptr);
    getQueue(QueueType::eGeneric).waitIdle();
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr Device::createBackedBuffer(std::string const& name,
    const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& properties,
    vk::DeviceSize dataSize, const void* data) const {

  auto result   = std::make_shared<BackedBuffer>();
  result->mName = name;

  result->mBufferInfo.size        = dataSize;
  result->mBufferInfo.usage       = usage;
  result->mBufferInfo.sharingMode = vk::SharingMode::eExclusive;

  // if data upload will use a staging buffer, we need to make sure transferDst is set!
  if ((data != nullptr) && (!(properties & vk::MemoryPropertyFlagBits::eHostVisible) ||
                               !(properties & vk::MemoryPropertyFlagBits::eHostCoherent))) {
    result->mBufferInfo.usage |= vk::BufferUsageFlagBits::eTransferDst;
  }

  result->mBuffer = createBuffer(name, result->mBufferInfo);

  auto requirements = mDevice->getBufferMemoryRequirements(*result->mBuffer);

  result->mMemoryInfo.allocationSize = requirements.size;
  result->mMemoryInfo.memoryTypeIndex =
      mPhysicalDevice->findMemoryType(requirements.memoryTypeBits, properties);

  result->mMemory = createMemory("Memory for " + name, result->mMemoryInfo);

  mDevice->bindBufferMemory(*result->mBuffer, *result->mMemory, 0);

  if (data != nullptr) {
    // data was provided, we need to upload it!
    if ((properties & vk::MemoryPropertyFlagBits::eHostVisible) &&
        (properties & vk::MemoryPropertyFlagBits::eHostCoherent)) {

      // simple case - memory is host visible and coherent;
      // we can simply map it and upload the data
      void* dst = mDevice->mapMemory(*result->mMemory, 0, dataSize);
      std::memcpy(dst, data, dataSize);
      mDevice->unmapMemory(*result->mMemory);
    } else {

      // more difficult case, we need a staging buffer!
      auto stagingBuffer =
          createBackedBuffer("StagingBuffer for " + name, vk::BufferUsageFlagBits::eTransferSrc,
              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
              dataSize, data);

      auto cmd = allocateCommandBuffer("Upload to BackedBuffer");
      cmd->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
      vk::BufferCopy region;
      region.size = dataSize;
      cmd->copyBuffer(*stagingBuffer->mBuffer, *result->mBuffer, 1, &region);
      cmd->end();

      vk::CommandBuffer bufs[] = {*cmd};
      vk::SubmitInfo    info;
      info.commandBufferCount = 1;
      info.pCommandBuffers    = bufs;

      getQueue(QueueType::eGeneric).submit(info, nullptr);
      getQueue(QueueType::eGeneric).waitIdle();
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr Device::createVertexBuffer(
    std::string const& name, vk::DeviceSize dataSize, const void* data) const {
  return createBackedBuffer(name, vk::BufferUsageFlagBits::eVertexBuffer,
      vk::MemoryPropertyFlagBits::eDeviceLocal, dataSize, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr Device::createIndexBuffer(
    std::string const& name, vk::DeviceSize dataSize, const void* data) const {
  return createBackedBuffer(name, vk::BufferUsageFlagBits::eIndexBuffer,
      vk::MemoryPropertyFlagBits::eDeviceLocal, dataSize, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr Device::createUniformBuffer(std::string const& name, vk::DeviceSize size) const {
  return createBackedBuffer(name,
      vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal, size);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Device::createTexture(std::string const& name, vk::ImageCreateInfo imageInfo,
    vk::SamplerCreateInfo samplerInfo, vk::ImageViewType viewType,
    const vk::ImageAspectFlags& imageAspectMask, vk::ImageLayout layout,
    vk::ComponentMapping const& componentMapping, vk::DeviceSize dataSize, const void* data) const {

  auto result = std::make_shared<Texture>();

  // create backed image for texture
  auto image = createBackedImage(name, std::move(imageInfo), viewType, imageAspectMask,
      vk::MemoryPropertyFlagBits::eDeviceLocal, layout, componentMapping, dataSize, data);

  result->mImage         = image->mImage;
  result->mImageInfo     = image->mImageInfo;
  result->mView          = image->mView;
  result->mViewInfo      = image->mViewInfo;
  result->mMemory        = image->mMemory;
  result->mMemoryInfo    = image->mMemoryInfo;
  result->mCurrentLayout = image->mCurrentLayout;

  // create sampler
  result->mSamplerInfo = std::move(samplerInfo);
  result->mSampler     = createSampler("Sampler for " + name, result->mSamplerInfo);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerCreateInfo Device::createSamplerInfo(
    vk::Filter filter, vk::SamplerMipmapMode mipmapMode, vk::SamplerAddressMode addressMode) {

  return vk::SamplerCreateInfo(
      vk::SamplerCreateFlags(), filter, filter, mipmapMode, addressMode, addressMode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Device::getSinglePixelTexture(std::array<uint8_t, 4> const& color) {

  auto cached = mSinglePixelTextures.find(color);
  if (cached != mSinglePixelTextures.end()) {
    return cached->second;
  }

  vk::ImageCreateInfo imageInfo;
  imageInfo.imageType     = vk::ImageType::e2D;
  imageInfo.format        = vk::Format::eR8G8B8A8Unorm;
  imageInfo.extent.width  = 1;
  imageInfo.extent.height = 1;
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = 1;
  imageInfo.samples       = vk::SampleCountFlagBits::e1;
  imageInfo.tiling        = vk::ImageTiling::eOptimal;
  imageInfo.usage         = vk::ImageUsageFlagBits::eSampled;
  imageInfo.sharingMode   = vk::SharingMode::eExclusive;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;

  vk::SamplerCreateInfo samplerInfo = createSamplerInfo();

  auto texture = createTexture("Single-pixel texture rgba(" + std::to_string(color[0]) + ", " +
                                   std::to_string(color[1]) + ", " + std::to_string(color[2]) +
                                   ", " + std::to_string(color[3]) + ")",
      imageInfo, samplerInfo, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::ComponentMapping(), 4, &color[0]);

  mSinglePixelTextures[color] = texture;

  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandBufferPtr Device::allocateCommandBuffer(
    std::string const& name, QueueType type, vk::CommandBufferLevel level) const {
  vk::CommandBufferAllocateInfo info;
  info.level              = level;
  info.commandPool        = *mCommandPools[Core::enumCast(type)];
  info.commandBufferCount = 1;

  Core::Logger::traceCreation("vk::CommandBuffer", name);

  auto vkObject = mDevice->allocateCommandBuffers(info)[0];
  assignName(uint64_t(VkCommandBuffer(vkObject)), vk::ObjectType::eCommandBuffer, name);

  auto device = mDevice;
  auto pool   = mCommandPools[Core::enumCast(type)];
  return VulkanPtr::create(vkObject, [device, pool, name](vk::CommandBuffer* obj) {
    Core::Logger::traceDeletion("vk::CommandBuffer", name);
    device->freeCommandBuffers(*pool, *obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::BufferPtr Device::createBuffer(
    std::string const& name, vk::BufferCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::Buffer", name);

  auto vkObject = mDevice->createBuffer(info);
  assignName(uint64_t(VkBuffer(vkObject)), vk::ObjectType::eBuffer, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Buffer* obj) {
    Core::Logger::traceDeletion("vk::Buffer", name);
    device->destroyBuffer(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandPoolPtr Device::createCommandPool(
    std::string const& name, vk::CommandPoolCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::CommandPool", name);

  auto vkObject = mDevice->createCommandPool(info);
  assignName(uint64_t(VkCommandPool(vkObject)), vk::ObjectType::eCommandPool, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::CommandPool* obj) {
    Core::Logger::traceDeletion("vk::CommandPool", name);
    device->destroyCommandPool(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorPoolPtr Device::createDescriptorPool(
    std::string const& name, vk::DescriptorPoolCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::DescriptorPool", name);

  auto vkObject = mDevice->createDescriptorPool(info);
  assignName(uint64_t(VkDescriptorPool(vkObject)), vk::ObjectType::eDescriptorPool, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::DescriptorPool* obj) {
    Core::Logger::traceDeletion("vk::DescriptorPool", name);
    device->destroyDescriptorPool(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorSetLayoutPtr Device::createDescriptorSetLayout(
    std::string const& name, vk::DescriptorSetLayoutCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::DescriptorSetLayout", name);

  auto vkObject = mDevice->createDescriptorSetLayout(info);
  assignName(uint64_t(VkDescriptorSetLayout(vkObject)), vk::ObjectType::eDescriptorSetLayout, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::DescriptorSetLayout* obj) {
    Core::Logger::traceDeletion("vk::DescriptorSetLayout", name);
    device->destroyDescriptorSetLayout(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DeviceMemoryPtr Device::createMemory(
    std::string const& name, vk::MemoryAllocateInfo const& info) const {
  Core::Logger::traceCreation("vk::DeviceMemory", name);

  auto vkObject = mDevice->allocateMemory(info);
  assignName(uint64_t(VkDeviceMemory(vkObject)), vk::ObjectType::eDeviceMemory, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::DeviceMemory* obj) {
    Core::Logger::traceDeletion("vk::DeviceMemory", name);
    device->freeMemory(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::FencePtr Device::createFence(std::string const& name, vk::FenceCreateFlags const& flags) const {
  Core::Logger::traceCreation("vk::Fence", name);

  auto vkObject = mDevice->createFence({flags});
  assignName(uint64_t(VkFence(vkObject)), vk::ObjectType::eFence, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Fence* obj) {
    Core::Logger::traceDeletion("vk::Fence", name);
    device->destroyFence(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::FramebufferPtr Device::createFramebuffer(
    std::string const& name, vk::FramebufferCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::Framebuffer", name);

  auto vkObject = mDevice->createFramebuffer(info);
  assignName(uint64_t(VkFramebuffer(vkObject)), vk::ObjectType::eFramebuffer, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Framebuffer* obj) {
    Core::Logger::traceDeletion("vk::Framebuffer", name);
    device->destroyFramebuffer(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ImagePtr Device::createImage(std::string const& name, vk::ImageCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::Image", name);

  auto vkObject = mDevice->createImage(info);
  assignName(uint64_t(VkImage(vkObject)), vk::ObjectType::eImage, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Image* obj) {
    Core::Logger::traceDeletion("vk::Image", name);
    device->destroyImage(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ImageViewPtr Device::createImageView(
    std::string const& name, vk::ImageViewCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::ImageView", name);

  auto vkObject = mDevice->createImageView(info);
  assignName(uint64_t(VkImageView(vkObject)), vk::ObjectType::eImageView, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::ImageView* obj) {
    Core::Logger::traceDeletion("vk::ImageView", name);
    device->destroyImageView(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelinePtr Device::createComputePipeline(
    std::string const& name, vk::ComputePipelineCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::Pipeline", name);

  auto vkObject = mDevice->createComputePipeline(nullptr, info);
  assignName(uint64_t(VkPipeline(vkObject)), vk::ObjectType::ePipeline, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Pipeline* obj) {
    Core::Logger::traceDeletion("vk::Pipeline", name);
    device->destroyPipeline(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelinePtr Device::createGraphicsPipeline(
    std::string const& name, vk::GraphicsPipelineCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::Pipeline", name);

  auto vkObject = mDevice->createGraphicsPipeline(nullptr, info);
  assignName(uint64_t(VkPipeline(vkObject)), vk::ObjectType::ePipeline, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Pipeline* obj) {
    Core::Logger::traceDeletion("vk::Pipeline", name);
    device->destroyPipeline(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelineLayoutPtr Device::createPipelineLayout(
    std::string const& name, vk::PipelineLayoutCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::PipelineLayout", name);

  auto vkObject = mDevice->createPipelineLayout(info);
  assignName(uint64_t(VkPipelineLayout(vkObject)), vk::ObjectType::ePipelineLayout, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::PipelineLayout* obj) {
    Core::Logger::traceDeletion("vk::PipelineLayout", name);
    device->destroyPipelineLayout(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::RenderPassPtr Device::createRenderPass(
    std::string const& name, vk::RenderPassCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::RenderPass", name);

  auto vkObject = mDevice->createRenderPass(info);
  assignName(uint64_t(VkRenderPass(vkObject)), vk::ObjectType::eRenderPass, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::RenderPass* obj) {
    Core::Logger::traceDeletion("vk::RenderPass", name);
    device->destroyRenderPass(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerPtr Device::createSampler(
    std::string const& name, vk::SamplerCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::Sampler", name);

  auto vkObject = mDevice->createSampler(info);
  assignName(uint64_t(VkSampler(vkObject)), vk::ObjectType::eSampler, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Sampler* obj) {
    Core::Logger::traceDeletion("vk::Sampler", name);
    device->destroySampler(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SemaphorePtr Device::createSemaphore(
    std::string const& name, vk::SemaphoreCreateFlags const& flags) const {
  Core::Logger::traceCreation("vk::Semaphore", name);

  auto vkObject = mDevice->createSemaphore({flags});
  assignName(uint64_t(VkSemaphore(vkObject)), vk::ObjectType::eSemaphore, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::Semaphore* obj) {
    Core::Logger::traceDeletion("vk::Semaphore", name);
    device->destroySemaphore(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ShaderModulePtr Device::createShaderModule(
    std::string const& name, vk::ShaderModuleCreateInfo const& info) const {
  Core::Logger::traceCreation("vk::ShaderModule", name);

  auto vkObject = mDevice->createShaderModule(info);
  assignName(uint64_t(VkShaderModule(vkObject)), vk::ObjectType::eShaderModule, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::ShaderModule* obj) {
    Core::Logger::traceDeletion("vk::ShaderModule", name);
    device->destroyShaderModule(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SwapchainKHRPtr Device::createSwapChainKhr(
    std::string const& name, vk::SwapchainCreateInfoKHR const& info) const {
  Core::Logger::traceCreation("vk::SwapchainKHR", name);

  auto vkObject = mDevice->createSwapchainKHR(info);
  assignName(uint64_t(VkSwapchainKHR(vkObject)), vk::ObjectType::eSwapchainKHR, name);

  auto device = mDevice;
  return VulkanPtr::create(vkObject, [device, name](vk::SwapchainKHR* obj) {
    Core::Logger::traceDeletion("vk::SwapchainKHR", name);
    device->destroySwapchainKHR(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DevicePtr const& Device::getHandle() const {
  return mDevice;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PhysicalDevicePtr const& Device::getPhysicalDevice() const {
  return mPhysicalDevice;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Queue const& Device::getQueue(QueueType type) const {
  return mQueues[Core::enumCast(type)];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Device::waitForFences(
    std::vector<vk::FencePtr> const& fences, bool waitAll, uint64_t timeout) {
  std::vector<vk::Fence> tmp(fences.size());
  for (size_t i(0); i < fences.size(); ++i) {
    tmp[i] = *fences[i];
  }
  mDevice->waitForFences(tmp, static_cast<vk::Bool32>(waitAll), timeout);
}

void Device::waitForFence(vk::FencePtr const& fence, uint64_t timeout) {
  mDevice->waitForFences(*fence, 1u, timeout);
}

void Device::resetFences(std::vector<vk::FencePtr> const& fences) {
  std::vector<vk::Fence> tmp(fences.size());
  for (size_t i(0); i < fences.size(); ++i) {
    tmp[i] = *fences[i];
  }
  mDevice->resetFences(tmp);
}

void Device::resetFence(vk::FencePtr const& fence) {
  mDevice->resetFences(*fence);
}

void Device::waitIdle() {
  mDevice->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DevicePtr Device::createDevice(std::string const& name) const {

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

  const float              queuePriority = 1.0f;
  const std::set<uint32_t> uniqueQueueFamilies{mPhysicalDevice->getQueueFamily(QueueType::eGeneric),
      mPhysicalDevice->getQueueFamily(QueueType::eCompute),
      mPhysicalDevice->getQueueFamily(QueueType::eTransfer)};

  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures;
  deviceFeatures.samplerAnisotropy = 1u;

  vk::DeviceCreateInfo createInfo;
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pEnabledFeatures        = &deviceFeatures;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
  createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

  Core::Logger::traceCreation("vk::Device", name);
  return VulkanPtr::create(mPhysicalDevice->createDevice(createInfo), [name](vk::Device* obj) {
    Core::Logger::traceDeletion("vk::Device", name);
    obj->destroy();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Device::assignName(
    uint64_t vulkanHandle, vk::ObjectType objectType, std::string const& name) const {
  vk::DebugUtilsObjectNameInfoEXT nameInfo;
  nameInfo.objectType   = objectType;
  nameInfo.objectHandle = vulkanHandle;
  nameInfo.pObjectName  = name.c_str();
  mSetObjectNameFunc(*mDevice, reinterpret_cast<VkDebugUtilsObjectNameInfoEXT*>(&nameInfo));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
