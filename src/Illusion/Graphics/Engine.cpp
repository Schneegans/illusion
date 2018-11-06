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
#include "Engine.hpp"

#include "../Core/Logger.hpp"
#include "PhysicalDevice.hpp"
#include "VulkanPtr.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <set>
#include <sstream>

namespace Illusion::Graphics {

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<const char*> VALIDATION_LAYERS{"VK_LAYER_LUNARG_standard_validation"};
const std::vector<const char*> DEVICE_EXTENSIONS{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

////////////////////////////////////////////////////////////////////////////////////////////////////

bool glfwInitialized{false};

////////////////////////////////////////////////////////////////////////////////////////////////////

VkBool32 messageCallback(
  VkDebugReportFlagsEXT      flags,
  VkDebugReportObjectTypeEXT type,
  uint64_t                   object,
  size_t                     location,
  int32_t                    code,
  const char*                layer,
  const char*                message,
  void*                      userData) {

  std::stringstream buf;
  buf << "[" << layer << "] " << message << " (code: " << code << ")" << std::endl;

  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    ILLUSION_ERROR << buf.str();
  } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    ILLUSION_WARNING << buf.str();
  } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    ILLUSION_TRACE << buf.str();
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool checkValidationLayerSupport() {
  for (auto const& layer : VALIDATION_LAYERS) {
    bool layerFound{false};

    for (auto const& property : vk::enumerateInstanceLayerProperties()) {
      if (std::strcmp(layer, property.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) { return false; }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<const char*> getRequiredInstanceExtensions(bool debugMode) {
  unsigned int glfwExtensionCount{0};
  const char** glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};

  std::vector<const char*> extensions;
  for (unsigned int i = 0; i < glfwExtensionCount; ++i) {
    extensions.push_back(glfwExtensions[i]);
  }

  if (debugMode) { extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); }

  return extensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::Engine(std::string const& app, bool debugMode)
  : mDebugMode(debugMode)
  , mInstance(createInstance("Illusion", app))
  , mDebugCallback(createDebugCallback())
  , mPhysicalDevice(createPhysicalDevice())
  , mDevice(createDevice())
  , mGraphicsQueue(mDevice->getQueue(mPhysicalDevice->getGraphicsFamily(), 0))
  , mComputeQueue(mDevice->getQueue(mPhysicalDevice->getComputeFamily(), 0))
  , mPresentQueue(mDevice->getQueue(mPhysicalDevice->getPresentFamily(), 0)) {

  ILLUSION_TRACE << "Creating Engine." << std::endl;
  {
    vk::CommandPoolCreateInfo info;
    info.queueFamilyIndex = mPhysicalDevice->getGraphicsFamily();
    info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    mGraphicsCommandPool = createCommandPool(info);
  }
  {
    vk::CommandPoolCreateInfo info;
    info.queueFamilyIndex = mPhysicalDevice->getComputeFamily();
    info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    mComputeCommandPool = createCommandPool(info);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::~Engine() {
  // FIXME: Material::clearPipelineCache();
  ILLUSION_TRACE << "Deleting Engine." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackedImage> Engine::createBackedImage(
  uint32_t                width,
  uint32_t                height,
  uint32_t                depth,
  uint32_t                levels,
  uint32_t                layers,
  vk::Format              format,
  vk::ImageTiling         tiling,
  vk::ImageUsageFlags     usage,
  vk::MemoryPropertyFlags properties,
  vk::SampleCountFlagBits samples,
  vk::ImageCreateFlags    flags) const {

  vk::ImageCreateInfo info;
  info.imageType     = vk::ImageType::e2D;
  info.extent.width  = width;
  info.extent.height = height;
  info.extent.depth  = depth;
  info.mipLevels     = levels;
  info.arrayLayers   = layers;
  info.format        = format;
  info.tiling        = tiling;
  info.initialLayout = vk::ImageLayout::eUndefined;
  info.usage         = usage;
  info.sharingMode   = vk::SharingMode::eExclusive;
  info.samples       = samples;
  info.flags         = flags;

  auto result = std::make_shared<BackedImage>();

  result->mImage = createImage(info);

  auto requirements = mDevice->getImageMemoryRequirements(*result->mImage);

  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex =
    mPhysicalDevice->findMemoryType(requirements.memoryTypeBits, properties);

  result->mMemory = createMemory(allocInfo);

  mDevice->bindImageMemory(*result->mImage, *result->mMemory, 0);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackedBuffer> Engine::createBackedBuffer(
  vk::DeviceSize          size,
  vk::BufferUsageFlags    usage,
  vk::MemoryPropertyFlags properties,
  void*                   data) const {

  auto result = std::make_shared<BackedBuffer>();

  {
    vk::BufferCreateInfo info;
    info.size        = size;
    info.usage       = usage;
    info.sharingMode = vk::SharingMode::eExclusive;

    // if data upload will use a staging buffer, we need to make sure transferDst is set!
    if (
      data && (!(properties & vk::MemoryPropertyFlagBits::eHostVisible) ||
               !(properties & vk::MemoryPropertyFlagBits::eHostCoherent))) {
      info.usage |= vk::BufferUsageFlagBits::eTransferDst;
    }

    result->mBuffer = createBuffer(info);
  }

  {
    auto requirements = mDevice->getBufferMemoryRequirements(*result->mBuffer);

    vk::MemoryAllocateInfo info;
    info.allocationSize  = requirements.size;
    info.memoryTypeIndex = mPhysicalDevice->findMemoryType(requirements.memoryTypeBits, properties);

    result->mMemory = createMemory(info);
  }

  mDevice->bindBufferMemory(*result->mBuffer, *result->mMemory, 0);

  if (data) {
    // data was provided, we need to upload it!
    if (
      (properties & vk::MemoryPropertyFlagBits::eHostVisible) &&
      (properties & vk::MemoryPropertyFlagBits::eHostCoherent)) {

      // simple case - memory is host visible and coherent;
      // we can simply map it and upload the data
      void* dst = mDevice->mapMemory(*result->mMemory, 0, size);
      std::memcpy(dst, data, size);
      mDevice->unmapMemory(*result->mMemory);
    } else {

      // more difficult case, we need a staging buffer!
      auto stagingBuffer = createBackedBuffer(
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        data);

      copyBuffer(stagingBuffer->mBuffer, result->mBuffer, size);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackedBuffer> Engine::createVertexBuffer(vk::DeviceSize size, void* data) const {
  return createBackedBuffer(
    size, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackedBuffer> Engine::createIndexBuffer(vk::DeviceSize size, void* data) const {
  return createBackedBuffer(
    size, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Buffer> Engine::createBuffer(vk::BufferCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Buffer." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createBuffer(info), [device](vk::Buffer* obj) {
    ILLUSION_TRACE << "Deleting vk::Buffer." << std::endl;
    device->destroyBuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::CommandPool> Engine::createCommandPool(
  vk::CommandPoolCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::CommandPool." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createCommandPool(info), [device](vk::CommandPool* obj) {
    ILLUSION_TRACE << "Deleting vk::CommandPool." << std::endl;
    device->destroyCommandPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorPool> Engine::createDescriptorPool(
  vk::DescriptorPoolCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::DescriptorPool." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createDescriptorPool(info), [device](vk::DescriptorPool* obj) {
    ILLUSION_TRACE << "Deleting vk::DescriptorPool." << std::endl;
    device->destroyDescriptorPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorSetLayout> Engine::createDescriptorSetLayout(
  vk::DescriptorSetLayoutCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::DescriptorSetLayout." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(
    device->createDescriptorSetLayout(info), [device](vk::DescriptorSetLayout* obj) {
      ILLUSION_TRACE << "Deleting vk::DescriptorSetLayout." << std::endl;
      device->destroyDescriptorSetLayout(*obj);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DeviceMemory> Engine::createMemory(vk::MemoryAllocateInfo const& info) const {
  ILLUSION_TRACE << "Allocating vk::DeviceMemory." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->allocateMemory(info), [device](vk::DeviceMemory* obj) {
    ILLUSION_TRACE << "Freeing vk::DeviceMemory." << std::endl;
    device->freeMemory(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Fence> Engine::createFence(vk::FenceCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Fence." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createFence(info), [device](vk::Fence* obj) {
    ILLUSION_TRACE << "Deleting vk::Fence." << std::endl;
    device->destroyFence(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Framebuffer> Engine::createFramebuffer(
  vk::FramebufferCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Framebuffer." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createFramebuffer(info), [device](vk::Framebuffer* obj) {
    ILLUSION_TRACE << "Deleting vk::Framebuffer." << std::endl;
    device->destroyFramebuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Image> Engine::createImage(vk::ImageCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Image." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createImage(info), [device](vk::Image* obj) {
    ILLUSION_TRACE << "Deleting vk::Image." << std::endl;
    device->destroyImage(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::ImageView> Engine::createImageView(vk::ImageViewCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::ImageView." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createImageView(info), [device](vk::ImageView* obj) {
    ILLUSION_TRACE << "Deleting vk::ImageView." << std::endl;
    device->destroyImageView(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Pipeline> Engine::createComputePipeline(
  vk::ComputePipelineCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::ComputePipeline." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createComputePipeline(nullptr, info), [device](vk::Pipeline* obj) {
    ILLUSION_TRACE << "Deleting vk::ComputePipeline." << std::endl;
    device->destroyPipeline(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Pipeline> Engine::createPipeline(
  vk::GraphicsPipelineCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Pipeline." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createGraphicsPipeline(nullptr, info), [device](vk::Pipeline* obj) {
    ILLUSION_TRACE << "Deleting vk::Pipeline." << std::endl;
    device->destroyPipeline(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::PipelineLayout> Engine::createPipelineLayout(
  vk::PipelineLayoutCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::PipelineLayout." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createPipelineLayout(info), [device](vk::PipelineLayout* obj) {
    ILLUSION_TRACE << "Deleting vk::PipelineLayout." << std::endl;
    device->destroyPipelineLayout(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::RenderPass> Engine::createRenderPass(
  vk::RenderPassCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::RenderPass." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createRenderPass(info), [device](vk::RenderPass* obj) {
    ILLUSION_TRACE << "Deleting vk::RenderPass." << std::endl;
    device->destroyRenderPass(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Sampler> Engine::createSampler(vk::SamplerCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Sampler." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createSampler(info), [device](vk::Sampler* obj) {
    ILLUSION_TRACE << "Deleting vk::Sampler." << std::endl;
    device->destroySampler(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Semaphore> Engine::createSemaphore(vk::SemaphoreCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::Semaphore." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createSemaphore(info), [device](vk::Semaphore* obj) {
    ILLUSION_TRACE << "Deleting vk::Semaphore." << std::endl;
    device->destroySemaphore(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::ShaderModule> Engine::createShaderModule(
  vk::ShaderModuleCreateInfo const& info) const {
  ILLUSION_TRACE << "Creating vk::ShaderModule." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createShaderModule(info), [device](vk::ShaderModule* obj) {
    ILLUSION_TRACE << "Deleting vk::ShaderModule." << std::endl;
    device->destroyShaderModule(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::SurfaceKHR> Engine::createSurface(GLFWwindow* window) const {
  VkSurfaceKHR tmp;
  if (glfwCreateWindowSurface(*mInstance, window, nullptr, &tmp) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create window surface!"};
  }

  ILLUSION_TRACE << "Creating vk::SurfaceKHR." << std::endl;

  // copying instance to keep reference counting up until the surface is destroyed
  auto instance{mInstance};
  return makeVulkanPtr(vk::SurfaceKHR(tmp), [instance](vk::SurfaceKHR* obj) {
    ILLUSION_TRACE << "Deleting vk::SurfaceKHR." << std::endl;
    instance->destroySurfaceKHR(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::SwapchainKHR> Engine::createSwapChainKhr(
  vk::SwapchainCreateInfoKHR const& info) const {
  ILLUSION_TRACE << "Creating vk::SwapchainKHR." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createSwapchainKHR(info), [device](vk::SwapchainKHR* obj) {
    ILLUSION_TRACE << "Deleting vk::SwapchainKHR." << std::endl;
    device->destroySwapchainKHR(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandBuffer Engine::beginSingleTimeGraphicsCommands() const {
  vk::CommandBufferAllocateInfo info;
  info.level              = vk::CommandBufferLevel::ePrimary;
  info.commandPool        = *mGraphicsCommandPool;
  info.commandBufferCount = 1;

  vk::CommandBuffer commandBuffer{mDevice->allocateCommandBuffers(info)[0]};

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::endSingleTimeGraphicsCommands(vk::CommandBuffer commandBuffer) const {
  commandBuffer.end();

  vk::SubmitInfo info;
  info.commandBufferCount = 1;
  info.pCommandBuffers    = &commandBuffer;

  mGraphicsQueue.submit(info, nullptr);
  mGraphicsQueue.waitIdle();

  mDevice->freeCommandBuffers(*mGraphicsCommandPool, commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandBuffer Engine::beginSingleTimeComputeCommands() const {
  vk::CommandBufferAllocateInfo info;
  info.level              = vk::CommandBufferLevel::ePrimary;
  info.commandPool        = *mComputeCommandPool;
  info.commandBufferCount = 1;

  vk::CommandBuffer commandBuffer{mDevice->allocateCommandBuffers(info)[0]};

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::endSingleTimeComputeCommands(vk::CommandBuffer commandBuffer) const {
  commandBuffer.end();

  vk::SubmitInfo info;
  info.commandBufferCount = 1;
  info.pCommandBuffers    = &commandBuffer;

  mComputeQueue.submit(info, nullptr);
  mComputeQueue.waitIdle();

  mDevice->freeCommandBuffers(*mComputeCommandPool, commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::transitionImageLayout(
  std::shared_ptr<vk::Image>& image,
  vk::ImageLayout             oldLayout,
  vk::ImageLayout             newLayout,
  vk::ImageSubresourceRange   subresourceRange) const {

  vk::CommandBuffer commandBuffer = beginSingleTimeGraphicsCommands();

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout           = oldLayout;
  barrier.newLayout           = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image               = *image;
  barrier.subresourceRange    = subresourceRange;

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (
    oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    sourceStage           = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage      = vk::PipelineStageFlagBits::eTransfer;
  } else if (
    oldLayout == vk::ImageLayout::eTransferDstOptimal &&
    newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    sourceStage           = vk::PipelineStageFlagBits::eTransfer;
    destinationStage      = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    ILLUSION_ERROR << "Requested an unsupported layout transition!" << std::endl;
  }

  commandBuffer.pipelineBarrier(
    sourceStage, destinationStage, vk::DependencyFlagBits(), nullptr, nullptr, barrier);

  endSingleTimeGraphicsCommands(commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::copyImage(
  std::shared_ptr<vk::Image>& src,
  std::shared_ptr<vk::Image>& dst,
  uint32_t                    width,
  uint32_t                    height) const {

  ILLUSION_TRACE << "Copying vk::Image." << std::endl;

  vk::CommandBuffer commandBuffer = beginSingleTimeGraphicsCommands();

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
  region.extent.width   = width;
  region.extent.height  = height;
  region.extent.depth   = 1;

  commandBuffer.copyImage(
    *src, vk::ImageLayout::eTransferSrcOptimal, *dst, vk::ImageLayout::eTransferDstOptimal, region);

  endSingleTimeGraphicsCommands(commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::copyBuffer(
  std::shared_ptr<vk::Buffer>& src, std::shared_ptr<vk::Buffer>& dst, vk::DeviceSize size) const {

  ILLUSION_TRACE << "Copying vk::Buffer." << std::endl;

  vk::CommandBuffer commandBuffer = beginSingleTimeGraphicsCommands();

  vk::BufferCopy region;
  region.size = size;

  commandBuffer.copyBuffer(*src, *dst, 1, &region);

  endSingleTimeGraphicsCommands(commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::isColorFormat(vk::Format format) {
  return !isDepthStencilFormat(format) && !isDepthOnlyFormat(format);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::isDepthFormat(vk::Format format) {
  return isDepthStencilFormat(format) || isDepthOnlyFormat(format);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::isDepthOnlyFormat(vk::Format format) {
  return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::isDepthStencilFormat(vk::Format format) {
  return format == vk::Format::eD16UnormS8Uint || format == vk::Format::eD24UnormS8Uint ||
         format == vk::Format::eD32SfloatS8Uint;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Instance> Engine::createInstance(
  std::string const& engine, std::string const& app) const {

  if (!glfwInitialized) {
    if (!glfwInit()) { throw std::runtime_error{"Failed to initialize GLFW."}; }

    glfwSetErrorCallback([](int error, const char* description) {
      throw std::runtime_error{"GLFW: " + std::string(description)};
    });

    glfwInitialized = true;
  }

  if (mDebugMode && !checkValidationLayerSupport()) {
    throw std::runtime_error{"Requested validation layers are not available!"};
  }

  // app info
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName   = app.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName        = engine.c_str();
  appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion         = VK_API_VERSION_1_0;

  // find required extensions
  auto extensions(getRequiredInstanceExtensions(mDebugMode));

  // create instance
  vk::InstanceCreateInfo info;
  info.pApplicationInfo        = &appInfo;
  info.enabledExtensionCount   = static_cast<int32_t>(extensions.size());
  info.ppEnabledExtensionNames = extensions.data();

  if (mDebugMode) {
    info.enabledLayerCount   = static_cast<int32_t>(VALIDATION_LAYERS.size());
    info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  } else {
    info.enabledLayerCount = 0;
  }

  ILLUSION_TRACE << "Creating vk::Instance." << std::endl;
  return makeVulkanPtr(vk::createInstance(info), [](vk::Instance* obj) {
    ILLUSION_TRACE << "Deleting vk::Instance." << std::endl;
    obj->destroy();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DebugReportCallbackEXT> Engine::createDebugCallback() const {
  if (!mDebugMode) { return nullptr; }

  auto createCallback{
    (PFN_vkCreateDebugReportCallbackEXT)mInstance->getProcAddr("vkCreateDebugReportCallbackEXT")};

  vk::DebugReportCallbackCreateInfoEXT info;
  info.flags = vk::DebugReportFlagBitsEXT::eInformation | vk::DebugReportFlagBitsEXT::eWarning |
               vk::DebugReportFlagBitsEXT::ePerformanceWarning |
               vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eDebug;
  info.pfnCallback = messageCallback;

  VkDebugReportCallbackEXT tmp;
  if (createCallback(*mInstance, (VkDebugReportCallbackCreateInfoEXT*)&info, nullptr, &tmp)) {
    throw std::runtime_error{"Failed to set up debug callback!"};
  }

  ILLUSION_TRACE << "Creating vk::DebugReportCallbackEXT." << std::endl;
  auto instance{mInstance};
  return makeVulkanPtr(
    vk::DebugReportCallbackEXT(tmp), [instance](vk::DebugReportCallbackEXT* obj) {
      auto destroyCallback = (PFN_vkDestroyDebugReportCallbackEXT)instance->getProcAddr(
        "vkDestroyDebugReportCallbackEXT");
      ILLUSION_TRACE << "Deleting vk::DebugReportCallbackEXT." << std::endl;
      destroyCallback(*instance, *obj, nullptr);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<PhysicalDevice> Engine::createPhysicalDevice() const {
  auto physicalDevices{mInstance->enumeratePhysicalDevices()};

  // loop through physical devices and choose a suitable one
  for (auto const& vkPhysicalDevice : physicalDevices) {

    auto physicalDevice = std::make_shared<PhysicalDevice>(*mInstance.get(), vkPhysicalDevice);

    // check whether all required queue types are supported
    if (
      physicalDevice->getGraphicsFamily() < 0 || physicalDevice->getPresentFamily() < 0 ||
      physicalDevice->getComputeFamily() < 0) {
      continue;
    }

    // check whether all required extensions are supported
    auto                  availableExtensions{physicalDevice->enumerateDeviceExtensionProperties()};
    std::set<std::string> requiredExtensions{DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end()};

    for (auto const& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    if (!requiredExtensions.empty()) { continue; }

    // all regquired extensions are supported - take this device!
    return physicalDevice;
  }

  throw std::runtime_error{"Failed to find a suitable vulkan device!"};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Device> Engine::createDevice() const {

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

  const float              queuePriority{1.0f};
  const std::set<uint32_t> uniqueQueueFamilies{(uint32_t)mPhysicalDevice->getGraphicsFamily(),
                                               (uint32_t)mPhysicalDevice->getComputeFamily(),
                                               (uint32_t)mPhysicalDevice->getPresentFamily()};

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
  return makeVulkanPtr(mPhysicalDevice->createDevice(createInfo), [](vk::Device* obj) {
    ILLUSION_TRACE << "Deleting vk::Device." << std::endl;
    obj->destroy();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Illusion::Graphics
