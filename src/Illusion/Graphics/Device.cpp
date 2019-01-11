////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
const std::vector<const char*> DEVICE_EXTENSIONS{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Device::Device(PhysicalDevicePtr const& physicalDevice)
    : mPhysicalDevice(physicalDevice)
    , mDevice(createDevice()) {

  ILLUSION_TRACE << "Creating Device." << std::endl;

  for (size_t i(0); i < 3; ++i) {
    QueueType type = static_cast<QueueType>(i);
    mQueues[i]     = mDevice->getQueue(mPhysicalDevice->getQueueFamily(type), 0);

    vk::CommandPoolCreateInfo info;
    info.queueFamilyIndex = mPhysicalDevice->getQueueFamily(type);
    info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    mCommandPools[i]      = createCommandPool(info);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Device::~Device() {
  ILLUSION_TRACE << "Deleting Device." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedImagePtr Device::createBackedImage(vk::ImageCreateInfo imageInfo, vk::ImageViewType viewType,
    vk::ImageAspectFlags imageAspectMask, vk::MemoryPropertyFlags properties,
    vk::ImageLayout layout, vk::ComponentMapping const& componentMapping, vk::DeviceSize dataSize,
    const void* data) const {

  auto result = std::make_shared<BackedImage>();

  // make sure eTransferDst is set when we have data to upload
  if (data) {
    imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
  }

  result->mImageInfo     = imageInfo;
  result->mImage         = createImage(imageInfo);
  result->mCurrentLayout = imageInfo.initialLayout;

  // create memory
  auto requirements = mDevice->getImageMemoryRequirements(*result->mImage);

  result->mMemoryInfo.allocationSize = requirements.size;
  result->mMemoryInfo.memoryTypeIndex =
      mPhysicalDevice->findMemoryType(requirements.memoryTypeBits, properties);
  result->mMemory = createMemory(result->mMemoryInfo);
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

  result->mView = createImageView(result->mViewInfo);

  if (data) {
    auto cmd = allocateCommandBuffer();
    cmd->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    auto stagingBuffer = createBackedBuffer(vk::BufferUsageFlagBits::eTransferSrc,
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

    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = layout;

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

  } else {

    auto cmd = allocateCommandBuffer();
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

BackedBufferPtr Device::createBackedBuffer(vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties, vk::DeviceSize dataSize, const void* data) const {

  auto result = std::make_shared<BackedBuffer>();

  result->mBufferInfo.size        = dataSize;
  result->mBufferInfo.usage       = usage;
  result->mBufferInfo.sharingMode = vk::SharingMode::eExclusive;

  // if data upload will use a staging buffer, we need to make sure transferDst is set!
  if (data && (!(properties & vk::MemoryPropertyFlagBits::eHostVisible) ||
                  !(properties & vk::MemoryPropertyFlagBits::eHostCoherent))) {
    result->mBufferInfo.usage |= vk::BufferUsageFlagBits::eTransferDst;
  }

  result->mBuffer = createBuffer(result->mBufferInfo);

  auto requirements = mDevice->getBufferMemoryRequirements(*result->mBuffer);

  result->mMemoryInfo.allocationSize = requirements.size;
  result->mMemoryInfo.memoryTypeIndex =
      mPhysicalDevice->findMemoryType(requirements.memoryTypeBits, properties);

  result->mMemory = createMemory(result->mMemoryInfo);

  mDevice->bindBufferMemory(*result->mBuffer, *result->mMemory, 0);

  if (data) {
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
      auto stagingBuffer = createBackedBuffer(vk::BufferUsageFlagBits::eTransferSrc,
          vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
          dataSize, data);

      auto cmd = allocateCommandBuffer();
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

BackedBufferPtr Device::createVertexBuffer(vk::DeviceSize dataSize, const void* data) const {
  return createBackedBuffer(vk::BufferUsageFlagBits::eVertexBuffer,
      vk::MemoryPropertyFlagBits::eDeviceLocal, dataSize, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr Device::createIndexBuffer(vk::DeviceSize dataSize, const void* data) const {
  return createBackedBuffer(vk::BufferUsageFlagBits::eIndexBuffer,
      vk::MemoryPropertyFlagBits::eDeviceLocal, dataSize, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr Device::createUniformBuffer(vk::DeviceSize size) const {
  return createBackedBuffer(
      vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal, size);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Device::createTexture(vk::ImageCreateInfo imageInfo, vk::SamplerCreateInfo samplerInfo,
    vk::ImageViewType viewType, vk::ImageAspectFlags imageAspectMask, vk::ImageLayout layout,
    vk::ComponentMapping const& componentMapping, vk::DeviceSize dataSize, const void* data) const {

  auto result = std::make_shared<Texture>();

  // create backed image for texture
  auto image = createBackedImage(imageInfo, viewType, imageAspectMask,
      vk::MemoryPropertyFlagBits::eDeviceLocal, layout, componentMapping, dataSize, data);

  result->mImage         = image->mImage;
  result->mImageInfo     = image->mImageInfo;
  result->mView          = image->mView;
  result->mViewInfo      = image->mViewInfo;
  result->mMemory        = image->mMemory;
  result->mMemoryInfo    = image->mMemoryInfo;
  result->mCurrentLayout = image->mCurrentLayout;

  // create sampler
  result->mSamplerInfo = samplerInfo;
  result->mSampler     = createSampler(result->mSamplerInfo);

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

  auto texture =
      createTexture(imageInfo, samplerInfo, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor,
          vk::ImageLayout::eShaderReadOnlyOptimal, vk::ComponentMapping(), 4, &color[0]);

  mSinglePixelTextures[color] = texture;

  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandBufferPtr Device::allocateCommandBuffer(
    QueueType type, vk::CommandBufferLevel level) const {
  vk::CommandBufferAllocateInfo info;
  info.level              = level;
  info.commandPool        = *mCommandPools[Core::enumCast(type)];
  info.commandBufferCount = 1;

  ILLUSION_TRACE << "Allocating CommandBuffer." << std::endl;

  auto device{mDevice};
  auto pool{mCommandPools[Core::enumCast(type)]};

  return VulkanPtr::create(
      mDevice->allocateCommandBuffers(info)[0], [device, pool](vk::CommandBuffer* obj) {
        ILLUSION_TRACE << "Freeing CommandBuffer." << std::endl;
        device->freeCommandBuffers(*pool, *obj);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::BufferPtr Device::createBuffer(vk::BufferCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Buffer." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createBuffer(info), [device](vk::Buffer* obj) {
    ILLUSION_TRACE << "Deleting vk::Buffer." << std::endl;
    device->destroyBuffer(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandPoolPtr Device::createCommandPool(vk::CommandPoolCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::CommandPool." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createCommandPool(info), [device](vk::CommandPool* obj) {
    ILLUSION_TRACE << "Deleting vk::CommandPool." << std::endl;
    device->destroyCommandPool(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorPoolPtr Device::createDescriptorPool(vk::DescriptorPoolCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::DescriptorPool." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createDescriptorPool(info), [device](vk::DescriptorPool* obj) {
    ILLUSION_TRACE << "Deleting vk::DescriptorPool." << std::endl;
    device->destroyDescriptorPool(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorSetLayoutPtr Device::createDescriptorSetLayout(
    vk::DescriptorSetLayoutCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::DescriptorSetLayout." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(
      device->createDescriptorSetLayout(info), [device](vk::DescriptorSetLayout* obj) {
        ILLUSION_TRACE << "Deleting vk::DescriptorSetLayout." << std::endl;
        device->destroyDescriptorSetLayout(*obj);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DeviceMemoryPtr Device::createMemory(vk::MemoryAllocateInfo const& info) const {
  ILLUSION_TRACE << "Allocating vk::DeviceMemory." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->allocateMemory(info), [device](vk::DeviceMemory* obj) {
    ILLUSION_TRACE << "Freeing vk::DeviceMemory." << std::endl;
    device->freeMemory(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::FencePtr Device::createFence(vk::FenceCreateFlags const& flags) const {
  ILLUSION_TRACE << "Creating vk::Fence." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createFence({flags}), [device](vk::Fence* obj) {
    ILLUSION_TRACE << "Deleting vk::Fence." << std::endl;
    device->destroyFence(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::FramebufferPtr Device::createFramebuffer(vk::FramebufferCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Framebuffer." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createFramebuffer(info), [device](vk::Framebuffer* obj) {
    ILLUSION_TRACE << "Deleting vk::Framebuffer." << std::endl;
    device->destroyFramebuffer(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ImagePtr Device::createImage(vk::ImageCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Image." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createImage(info), [device](vk::Image* obj) {
    ILLUSION_TRACE << "Deleting vk::Image." << std::endl;
    device->destroyImage(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ImageViewPtr Device::createImageView(vk::ImageViewCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::ImageView." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createImageView(info), [device](vk::ImageView* obj) {
    ILLUSION_TRACE << "Deleting vk::ImageView." << std::endl;
    device->destroyImageView(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelinePtr Device::createComputePipeline(vk::ComputePipelineCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Pipeline (compute)." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(
      device->createComputePipeline(nullptr, info), [device](vk::Pipeline* obj) {
        ILLUSION_TRACE << "Deleting vk::Pipeline (compute)." << std::endl;
        device->destroyPipeline(*obj);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelinePtr Device::createGraphicsPipeline(vk::GraphicsPipelineCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Pipeline (graphics)." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(
      device->createGraphicsPipeline(nullptr, info), [device](vk::Pipeline* obj) {
        ILLUSION_TRACE << "Deleting vk::Pipeline (graphics)." << std::endl;
        device->destroyPipeline(*obj);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelineLayoutPtr Device::createPipelineLayout(vk::PipelineLayoutCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::PipelineLayout." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createPipelineLayout(info), [device](vk::PipelineLayout* obj) {
    ILLUSION_TRACE << "Deleting vk::PipelineLayout." << std::endl;
    device->destroyPipelineLayout(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::RenderPassPtr Device::createRenderPass(vk::RenderPassCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::RenderPass." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createRenderPass(info), [device](vk::RenderPass* obj) {
    ILLUSION_TRACE << "Deleting vk::RenderPass." << std::endl;
    device->destroyRenderPass(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerPtr Device::createSampler(vk::SamplerCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Sampler." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createSampler(info), [device](vk::Sampler* obj) {
    ILLUSION_TRACE << "Deleting vk::Sampler." << std::endl;
    device->destroySampler(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SemaphorePtr Device::createSemaphore(vk::SemaphoreCreateFlags const& flags) const {
  ILLUSION_TRACE << "Creating vk::Semaphore." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createSemaphore({flags}), [device](vk::Semaphore* obj) {
    ILLUSION_TRACE << "Deleting vk::Semaphore." << std::endl;
    device->destroySemaphore(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ShaderModulePtr Device::createShaderModule(vk::ShaderModuleCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::ShaderModule." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createShaderModule(info), [device](vk::ShaderModule* obj) {
    ILLUSION_TRACE << "Deleting vk::ShaderModule." << std::endl;
    device->destroyShaderModule(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SwapchainKHRPtr Device::createSwapChainKhr(vk::SwapchainCreateInfoKHR const& info) const {
  ILLUSION_TRACE << "Creating vk::SwapchainKHR." << std::endl;
  auto device{mDevice};
  return VulkanPtr::create(device->createSwapchainKHR(info), [device](vk::SwapchainKHR* obj) {
    ILLUSION_TRACE << "Deleting vk::SwapchainKHR." << std::endl;
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
    vk::ArrayProxy<const vk::Fence> const& fences, bool waitAll, uint64_t timeout) {
  mDevice->waitForFences(fences, waitAll, timeout);
}

void Device::resetFences(vk::ArrayProxy<const vk::Fence> const& fences) {
  mDevice->resetFences(fences);
}

void Device::waitIdle() {
  mDevice->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DevicePtr Device::createDevice() const {

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

  const float              queuePriority{1.0f};
  const std::set<uint32_t> uniqueQueueFamilies{
      (uint32_t)mPhysicalDevice->getQueueFamily(QueueType::eGeneric),
      (uint32_t)mPhysicalDevice->getQueueFamily(QueueType::eCompute),
      (uint32_t)mPhysicalDevice->getQueueFamily(QueueType::eTransfer)};

  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures;
  deviceFeatures.samplerAnisotropy = true;

  vk::DeviceCreateInfo createInfo;
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.queueCreateInfoCount    = (uint32_t)queueCreateInfos.size();
  createInfo.pEnabledFeatures        = &deviceFeatures;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
  createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

  ILLUSION_TRACE << "Creating vk::Device." << std::endl;
  return VulkanPtr::create(mPhysicalDevice->createDevice(createInfo), [](vk::Device* obj) {
    ILLUSION_TRACE << "Deleting vk::Device." << std::endl;
    obj->destroy();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
