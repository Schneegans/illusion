////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DEVICE_HPP
#define ILLUSION_GRAPHICS_DEVICE_HPP

#include "../Core/BitHash.hpp"
#include "fwd.hpp"

#include <glm/glm.hpp>
#include <map>

struct GLFWwindow;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedImage {
  vk::ImagePtr        mImage;
  vk::ImageCreateInfo mImageInfo;

  vk::ImageViewPtr        mView;
  vk::ImageViewCreateInfo mViewInfo;

  vk::DeviceMemoryPtr    mMemory;
  vk::MemoryAllocateInfo mMemoryInfo;

  vk::ImageLayout mCurrentLayout = vk::ImageLayout::eUndefined;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct Texture {
  std::shared_ptr<BackedImage> mBackedImage;

  vk::SamplerPtr        mSampler;
  vk::SamplerCreateInfo mSamplerInfo;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedBuffer {
  vk::BufferPtr        mBuffer;
  vk::BufferCreateInfo mBufferInfo;

  vk::DeviceMemoryPtr    mMemory;
  vk::MemoryAllocateInfo mMemoryInfo;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class Device {

 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static DevicePtr create(Args&&... args) {
    return std::make_shared<Device>(args...);
  };

  explicit Device(PhysicalDevicePtr const& physicalDevice);
  virtual ~Device();

  // high-level create methods ---------------------------------------------------------------------
  BackedImagePtr createBackedImage(vk::ImageCreateInfo info, vk::ImageViewType viewType,
      vk::ImageAspectFlags imageAspectMask, vk::MemoryPropertyFlags properties,
      vk::ImageLayout layout, vk::ComponentMapping const& componentMapping = vk::ComponentMapping(),
      vk::DeviceSize dataSize = 0, const void* data = nullptr) const;

  BackedBufferPtr createBackedBuffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
      vk::DeviceSize dataSize, const void* data = nullptr) const;

  BackedBufferPtr createVertexBuffer(vk::DeviceSize dataSize, const void* data) const;
  BackedBufferPtr createIndexBuffer(vk::DeviceSize dataSize, const void* data) const;

  template <typename T>
  BackedBufferPtr createVertexBuffer(T const& data) const {
    return createVertexBuffer(sizeof(typename T::value_type) * data.size(), data.data());
  }

  template <typename T>
  BackedBufferPtr createIndexBuffer(T const& data) const {
    return createIndexBuffer(sizeof(typename T::value_type) * data.size(), data.data());
  }

  BackedBufferPtr createUniformBuffer(vk::DeviceSize size) const;

  TexturePtr createTexture(vk::ImageCreateInfo imageInfo, vk::SamplerCreateInfo samplerInfo,
      vk::ImageViewType viewType, vk::ImageAspectFlags imageAspectMask, vk::ImageLayout layout,
      vk::ComponentMapping const& componentMapping = vk::ComponentMapping(),
      vk::DeviceSize dataSize = 0, const void* data = nullptr) const;

  TexturePtr getSinglePixelTexture(std::array<uint8_t, 4> const& color);

  static vk::SamplerCreateInfo createSamplerInfo(vk::Filter filter = vk::Filter::eLinear,
      vk::SamplerMipmapMode  mipmapMode                            = vk::SamplerMipmapMode::eLinear,
      vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eClampToEdge);

  // low-level create methods ----------------------------------------------------------------------
  // clang-format off
  vk::CommandBufferPtr       allocateCommandBuffer(QueueType = QueueType::eGeneric, vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary) const;
  vk::BufferPtr              createBuffer(vk::BufferCreateInfo const&) const;
  vk::CommandPoolPtr         createCommandPool(vk::CommandPoolCreateInfo const&) const;
  vk::DescriptorPoolPtr      createDescriptorPool(vk::DescriptorPoolCreateInfo const&) const;
  vk::DescriptorSetLayoutPtr createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const&) const;
  vk::DeviceMemoryPtr        createMemory(vk::MemoryAllocateInfo const&) const;
  vk::FencePtr               createFence(vk::FenceCreateFlags const& = vk::FenceCreateFlagBits::eSignaled) const;
  vk::FramebufferPtr         createFramebuffer(vk::FramebufferCreateInfo const&) const;
  vk::ImagePtr               createImage(vk::ImageCreateInfo const&) const;
  vk::ImageViewPtr           createImageView(vk::ImageViewCreateInfo const&) const;
  vk::PipelinePtr            createComputePipeline(vk::ComputePipelineCreateInfo const&) const;
  vk::PipelinePtr            createGraphicsPipeline(vk::GraphicsPipelineCreateInfo const&) const;
  vk::PipelineLayoutPtr      createPipelineLayout(vk::PipelineLayoutCreateInfo const&) const;
  vk::RenderPassPtr          createRenderPass(vk::RenderPassCreateInfo const&) const;
  vk::SamplerPtr             createSampler(vk::SamplerCreateInfo const&) const;
  vk::SemaphorePtr           createSemaphore(vk::SemaphoreCreateFlags const& = {}) const;
  vk::ShaderModulePtr        createShaderModule(vk::ShaderModuleCreateInfo const&) const;
  vk::SwapchainKHRPtr        createSwapChainKhr(vk::SwapchainCreateInfoKHR const&) const;
  // clang-format on

  // vulkan getters --------------------------------------------------------------------------------
  vk::DevicePtr const& getHandle() const {
    return mDevice;
  }
  PhysicalDevicePtr const& getPhysicalDevice() const {
    return mPhysicalDevice;
  }
  vk::Queue const& getQueue(QueueType type) const;

  // device interface forwarding -------------------------------------------------------------------
  void waitForFences(vk::ArrayProxy<const vk::Fence> const& fences, bool waitAll, uint64_t timeout);
  void resetFences(vk::ArrayProxy<const vk::Fence> const& fences);
  void waitIdle();

 private:
  vk::DevicePtr createDevice() const;

  PhysicalDevicePtr mPhysicalDevice;
  vk::DevicePtr     mDevice;

  std::array<vk::Queue, 3>          mQueues;
  std::array<vk::CommandPoolPtr, 3> mCommandPools;

  std::map<std::array<uint8_t, 4>, TexturePtr> mSinglePixelTextures;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DEVICE_HPP
