////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_CONTEXT_HPP
#define ILLUSION_GRAPHICS_CONTEXT_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Core/BitHash.hpp"
#include "fwd.hpp"

#include <map>

struct GLFWwindow;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedImage {
  std::shared_ptr<vk::Image>        mImage;
  std::shared_ptr<vk::DeviceMemory> mMemory;
  vk::DeviceSize                    mSize;
};

struct BackedBuffer {
  std::shared_ptr<vk::Buffer>       mBuffer;
  std::shared_ptr<vk::DeviceMemory> mMemory;
  vk::DeviceSize                    mSize;
};

// -------------------------------------------------------------------------------------------------
class Context {

 public:
  // -------------------------------------------------------------------------------- public methods
  Context(std::shared_ptr<PhysicalDevice> const& physicalDevice);
  virtual ~Context();

  // --------------------------------------------------------------------- high-level create methods
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

  std::shared_ptr<CommandBuffer> allocateGraphicsCommandBuffer() const;
  std::shared_ptr<CommandBuffer> allocateComputeCommandBuffer() const;

  // ---------------------------------------------------------------------- low-level create methods
  // clang-format off
  std::shared_ptr<vk::Buffer>              createBuffer(vk::BufferCreateInfo const&) const;
  std::shared_ptr<vk::CommandPool>         createCommandPool(vk::CommandPoolCreateInfo const&) const;
  std::shared_ptr<vk::DescriptorPool>      createDescriptorPool(vk::DescriptorPoolCreateInfo const&) const;
  std::shared_ptr<vk::DescriptorSetLayout> createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const&) const;
  std::shared_ptr<vk::DeviceMemory>        createMemory(vk::MemoryAllocateInfo const&) const;
  std::shared_ptr<vk::Fence>               createFence(vk::FenceCreateInfo const&) const;
  std::shared_ptr<vk::Framebuffer>         createFramebuffer(vk::FramebufferCreateInfo const&) const;
  std::shared_ptr<vk::Image>               createImage(vk::ImageCreateInfo const&) const;
  std::shared_ptr<vk::ImageView>           createImageView(vk::ImageViewCreateInfo const&) const;
  std::shared_ptr<vk::Pipeline>            createComputePipeline(vk::ComputePipelineCreateInfo const&) const;
  std::shared_ptr<vk::Pipeline>            createPipeline(vk::GraphicsPipelineCreateInfo const&) const;
  std::shared_ptr<vk::PipelineLayout>      createPipelineLayout(vk::PipelineLayoutCreateInfo const&) const;
  std::shared_ptr<vk::RenderPass>          createRenderPass(vk::RenderPassCreateInfo const&) const;
  std::shared_ptr<vk::Sampler>             createSampler(vk::SamplerCreateInfo const&) const;
  std::shared_ptr<vk::Semaphore>           createSemaphore(vk::SemaphoreCreateInfo const&) const;
  std::shared_ptr<vk::ShaderModule>        createShaderModule(vk::ShaderModuleCreateInfo const&) const;
  std::shared_ptr<vk::SwapchainKHR>        createSwapChainKhr(vk::SwapchainCreateInfoKHR const&) const;

  // clang-format on

  // ------------------------------------------------------------------------- vulkan helper methods
  std::shared_ptr<CommandBuffer> beginSingleTimeGraphicsCommands() const;
  void endSingleTimeGraphicsCommands(std::shared_ptr<CommandBuffer> commandBuffer) const;

  std::shared_ptr<CommandBuffer> beginSingleTimeComputeCommands() const;
  void endSingleTimeComputeCommands(std::shared_ptr<CommandBuffer> commandBuffer) const;

  void transitionImageLayout(
    std::shared_ptr<vk::Image>& image,
    vk::ImageLayout             oldLayout,
    vk::ImageLayout             newLayout,
    vk::ImageSubresourceRange   range) const;

  void copyImage(
    std::shared_ptr<vk::Image>& src,
    std::shared_ptr<vk::Image>& dst,
    uint32_t                    width,
    uint32_t                    height) const;

  void copyBuffer(
    std::shared_ptr<vk::Buffer>& src, std::shared_ptr<vk::Buffer>& dst, vk::DeviceSize size) const;

  // -------------------------------------------------------------------------------- vulkan getters
  std::shared_ptr<vk::Device> const&     getDevice() const { return mDevice; }
  std::shared_ptr<PhysicalDevice> const& getPhysicalDevice() const { return mPhysicalDevice; }
  vk::Queue const&                       getGraphicsQueue() const { return mGraphicsQueue; }
  vk::Queue const&                       getComputeQueue() const { return mComputeQueue; }
  vk::Queue const&                       getPresentQueue() const { return mPresentQueue; }

  std::shared_ptr<vk::CommandPool> const& getGraphicsCommandPool() const {
    return mGraphicsCommandPool;
  }
  std::shared_ptr<vk::CommandPool> const& getComputeCommandPool() const {
    return mComputeCommandPool;
  }

 private:
  // ------------------------------------------------------------------------------- private methods
  std::shared_ptr<vk::Device> createDevice() const;

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<PhysicalDevice>  mPhysicalDevice;
  std::shared_ptr<vk::Device>      mDevice;
  std::shared_ptr<vk::CommandPool> mGraphicsCommandPool;
  std::shared_ptr<vk::CommandPool> mComputeCommandPool;

  vk::Queue mGraphicsQueue, mComputeQueue, mPresentQueue;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_CONTEXT_HPP
