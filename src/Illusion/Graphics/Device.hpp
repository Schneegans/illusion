////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DEVICE_HPP
#define ILLUSION_GRAPHICS_DEVICE_HPP

#include "../Core/BitHash.hpp"
#include "../Core/NamedObject.hpp"
#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

#include <glm/glm.hpp>
#include <map>

struct GLFWwindow;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Device is your main entry for creating Vulkan objects. Usually you will have exactly one   //
// Device for your application.                                                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Device : public Core::StaticCreate<Device>, public Core::NamedObject {

 public:
  // The device needs the physical device it should be created for. You can get one from your
  // Instance.
  explicit Device(std::string const& name, PhysicalDevicePtr const& physicalDevice);
  virtual ~Device();

  // high-level create methods ---------------------------------------------------------------------
  // These methods will usually create multiple Vulkan resources and upload data to the GPU by
  // issuing CommandBuffers. They may even allocate temporary objects such as staging buffers.

  // Creates a BackedImage and optionally uploads data to the GPU. This uses a BackedBuffer as
  // staging buffer.
  BackedImagePtr createBackedImage(std::string const& name, vk::ImageCreateInfo info,
      vk::ImageViewType viewType, vk::ImageAspectFlags imageAspectMask,
      vk::MemoryPropertyFlags properties, vk::ImageLayout layout,
      vk::ComponentMapping const& componentMapping = vk::ComponentMapping(),
      vk::DeviceSize dataSize = 0, const void* data = nullptr) const;

  // Creates a BackedBuffer and optionally uploads data to the GPU. If the memory is eHostVisible
  // and eHostCoherent, the data will be uploaded by mapping. Else a staging buffer will be used.
  BackedBufferPtr createBackedBuffer(std::string const& name, vk::BufferUsageFlags usage,
      vk::MemoryPropertyFlags properties, vk::DeviceSize dataSize,
      const void* data = nullptr) const;

  // Creates a device-local BackedBuffer with vk::BufferUsageFlagBits::eVertexBuffer and uploads the
  // given data. You may use the convenience template-version below to directly upload objects such
  // as structs.
  BackedBufferPtr createVertexBuffer(
      std::string const& name, vk::DeviceSize dataSize, const void* data) const;

  template <typename T>
  BackedBufferPtr createVertexBuffer(std::string const& name, T const& data) const {
    return createVertexBuffer(name, sizeof(typename T::value_type) * data.size(), data.data());
  }

  // Creates a device-local BackedBuffer with vk::BufferUsageFlagBits::eIndexBuffer and uploads the
  // given data. You may use the convenience template-version below to directly upload objects such
  // as structs.
  BackedBufferPtr createIndexBuffer(
      std::string const& name, vk::DeviceSize dataSize, const void* data) const;

  template <typename T>
  BackedBufferPtr createIndexBuffer(std::string const& name, T const& data) const {
    return createIndexBuffer(name, sizeof(typename T::value_type) * data.size(), data.data());
  }

  // Creates a device-local BackedBuffer with vk::BufferUsageFlagBits::eUniformBuffer and
  // vk::BufferUsageFlagBits::eTransferDst.
  BackedBufferPtr createUniformBuffer(std::string const& name, vk::DeviceSize size) const;

  TexturePtr createTexture(std::string const& name, vk::ImageCreateInfo imageInfo,
      vk::SamplerCreateInfo samplerInfo, vk::ImageViewType viewType,
      vk::ImageAspectFlags imageAspectMask, vk::ImageLayout layout,
      vk::ComponentMapping const& componentMapping = vk::ComponentMapping(),
      vk::DeviceSize dataSize = 0, const void* data = nullptr) const;

  // If you need a texture with a single pixel of a specific color, you can use this method. When
  // called multiple times with the same color, it will only create a texture once.
  TexturePtr getSinglePixelTexture(std::array<uint8_t, 4> const& color);

  // static method for easy allocation of a vk::SamplerCreateInfo. It uses useful defaults and
  // assigns the same filter to magFilter and minFilter as well as the same address mode to U, V and
  // W.
  static vk::SamplerCreateInfo createSamplerInfo(vk::Filter filter = vk::Filter::eLinear,
      vk::SamplerMipmapMode  mipmapMode                            = vk::SamplerMipmapMode::eLinear,
      vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eClampToEdge);

  // low-level create methods ----------------------------------------------------------------------
  // You should use these low-level methods to create Vulkan resources. They are wrapped in a
  // std::shared_ptr which tracks the object they were created by (usually the internal vk::Device,
  // but for example a vk::CommandBufferPtr will also capture the vk::CommandPoolPtr it was
  // allocated from). This ensures that an object will not be deleted before all dependent objects
  // are deleted.

  // clang-format off
  vk::CommandBufferPtr       allocateCommandBuffer(std::string const& name, QueueType = QueueType::eGeneric, vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary) const;
  vk::BufferPtr              createBuffer(std::string const& name, vk::BufferCreateInfo const&) const;
  vk::CommandPoolPtr         createCommandPool(std::string const& name, vk::CommandPoolCreateInfo const&) const;
  vk::DescriptorPoolPtr      createDescriptorPool(std::string const& name, vk::DescriptorPoolCreateInfo const&) const;
  vk::DescriptorSetLayoutPtr createDescriptorSetLayout(std::string const& name, vk::DescriptorSetLayoutCreateInfo const&) const;
  vk::DeviceMemoryPtr        createMemory(std::string const& name, vk::MemoryAllocateInfo const&) const;
  vk::FencePtr               createFence(std::string const& name, vk::FenceCreateFlags const& = vk::FenceCreateFlagBits::eSignaled) const;
  vk::FramebufferPtr         createFramebuffer(std::string const& name, vk::FramebufferCreateInfo const&) const;
  vk::ImagePtr               createImage(std::string const& name, vk::ImageCreateInfo const&) const;
  vk::ImageViewPtr           createImageView(std::string const& name, vk::ImageViewCreateInfo const&) const;
  vk::PipelinePtr            createComputePipeline(std::string const& name, vk::ComputePipelineCreateInfo const&) const;
  vk::PipelinePtr            createGraphicsPipeline(std::string const& name, vk::GraphicsPipelineCreateInfo const&) const;
  vk::PipelineLayoutPtr      createPipelineLayout(std::string const& name, vk::PipelineLayoutCreateInfo const&) const;
  vk::RenderPassPtr          createRenderPass(std::string const& name, vk::RenderPassCreateInfo const&) const;
  vk::SamplerPtr             createSampler(std::string const& name, vk::SamplerCreateInfo const&) const;
  vk::SemaphorePtr           createSemaphore(std::string const& name, vk::SemaphoreCreateFlags const& = {}) const;
  vk::ShaderModulePtr        createShaderModule(std::string const& name, vk::ShaderModuleCreateInfo const&) const;
  vk::SwapchainKHRPtr        createSwapChainKhr(std::string const& name, vk::SwapchainCreateInfoKHR const&) const;
  // clang-format on

  // vulkan getters --------------------------------------------------------------------------------
  vk::DevicePtr const&     getHandle() const;
  PhysicalDevicePtr const& getPhysicalDevice() const;
  vk::Queue const&         getQueue(QueueType type) const;

  // device interface forwarding -------------------------------------------------------------------
  void waitForFences(
      std::vector<vk::FencePtr> const& fences, bool waitAll = true, uint64_t timeout = ~0);
  void waitForFence(vk::FencePtr const& fence, uint64_t timeout = ~0);
  void resetFences(std::vector<vk::FencePtr> const& fences);
  void resetFence(vk::FencePtr const& fence);
  void waitIdle();

 private:
  vk::DevicePtr createDevice() const;
  void assignName(uint64_t vulkanHandle, vk::ObjectType objectType, std::string const& name) const;

  PhysicalDevicePtr mPhysicalDevice;
  vk::DevicePtr     mDevice;

  PFN_vkSetDebugUtilsObjectNameEXT mSetObjectNameFunc;

  // One for each QueueType
  std::array<vk::Queue, 3>          mQueues;
  std::array<vk::CommandPoolPtr, 3> mCommandPools;

  std::map<std::array<uint8_t, 4>, TexturePtr> mSinglePixelTextures;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DEVICE_HPP
