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
////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedImage {
  std::shared_ptr<vk::Image>        mImage;
  std::shared_ptr<vk::DeviceMemory> mMemory;
  vk::DeviceSize                    mSize;
  vk::ImageCreateInfo               mInfo;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedBuffer {
  std::shared_ptr<vk::Buffer>       mBuffer;
  std::shared_ptr<vk::DeviceMemory> mMemory;
  vk::DeviceSize                    mSize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class Device {

 public:
  explicit Device(std::shared_ptr<PhysicalDevice> const& physicalDevice);
  virtual ~Device();

  // high-level create methods ---------------------------------------------------------------------
  std::shared_ptr<BackedImage> createBackedImage(
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
    vk::ImageCreateFlags    flags = vk::ImageCreateFlags()) const;

  std::shared_ptr<BackedBuffer> createBackedBuffer(
    vk::DeviceSize          size,
    vk::BufferUsageFlags    usage,
    vk::MemoryPropertyFlags properties,
    const void*             data = nullptr) const;

  std::shared_ptr<BackedBuffer> createVertexBuffer(vk::DeviceSize size, const void* data) const;
  std::shared_ptr<BackedBuffer> createIndexBuffer(vk::DeviceSize size, const void* data) const;
  std::shared_ptr<BackedBuffer> createUniformBuffer(vk::DeviceSize size) const;

  // low-level create methods ----------------------------------------------------------------------
  // clang-format off
  std::shared_ptr<vk::CommandBuffer>       allocateCommandBuffer(QueueType = QueueType::eGeneric, vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary) const;
  std::shared_ptr<vk::Buffer>              createBuffer(vk::BufferCreateInfo const&) const;
  std::shared_ptr<vk::CommandPool>         createCommandPool(vk::CommandPoolCreateInfo const&) const;
  std::shared_ptr<vk::DescriptorPool>      createDescriptorPool(vk::DescriptorPoolCreateInfo const&) const;
  std::shared_ptr<vk::DescriptorSetLayout> createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const&) const;
  std::shared_ptr<vk::DeviceMemory>        createMemory(vk::MemoryAllocateInfo const&) const;
  std::shared_ptr<vk::Fence>               createFence(vk::FenceCreateFlags const& = vk::FenceCreateFlagBits::eSignaled) const;
  std::shared_ptr<vk::Framebuffer>         createFramebuffer(vk::FramebufferCreateInfo const&) const;
  std::shared_ptr<vk::Image>               createImage(vk::ImageCreateInfo const&) const;
  std::shared_ptr<vk::ImageView>           createImageView(vk::ImageViewCreateInfo const&) const;
  std::shared_ptr<vk::Pipeline>            createComputePipeline(vk::ComputePipelineCreateInfo const&) const;
  std::shared_ptr<vk::Pipeline>            createPipeline(vk::GraphicsPipelineCreateInfo const&) const;
  std::shared_ptr<vk::PipelineLayout>      createPipelineLayout(vk::PipelineLayoutCreateInfo const&) const;
  std::shared_ptr<vk::RenderPass>          createRenderPass(vk::RenderPassCreateInfo const&) const;
  std::shared_ptr<vk::Sampler>             createSampler(vk::SamplerCreateInfo const&) const;
  std::shared_ptr<vk::Semaphore>           createSemaphore(vk::SemaphoreCreateFlags const& = {}) const;
  std::shared_ptr<vk::ShaderModule>        createShaderModule(vk::ShaderModuleCreateInfo const&) const;
  std::shared_ptr<vk::SwapchainKHR>        createSwapChainKhr(vk::SwapchainCreateInfoKHR const&) const;
  // clang-format on

  // vulkan getters --------------------------------------------------------------------------------
  std::shared_ptr<vk::Device> const&     getHandle() const { return mDevice; }
  std::shared_ptr<PhysicalDevice> const& getPhysicalDevice() const { return mPhysicalDevice; }
  vk::Queue const&                       getQueue(QueueType type) const;

  // device interface forwarding -------------------------------------------------------------------
  void waitForFences(vk::ArrayProxy<const vk::Fence> const& fences, bool waitAll, uint64_t timeout);
  void resetFences(vk::ArrayProxy<const vk::Fence> const& fences);
  void waitIdle();

 private:
  std::shared_ptr<vk::Device> createDevice() const;

  std::shared_ptr<PhysicalDevice> mPhysicalDevice;
  std::shared_ptr<vk::Device>     mDevice;

  std::array<vk::Queue, 3>                        mQueues;
  std::array<std::shared_ptr<vk::CommandPool>, 3> mCommandPools;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DEVICE_HPP
