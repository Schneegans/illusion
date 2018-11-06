////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_ENGINE_HPP
#define ILLUSION_GRAPHICS_ENGINE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

struct GLFWwindow;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedImage {
  std::shared_ptr<vk::Image>        mImage;
  std::shared_ptr<vk::DeviceMemory> mMemory;
};

struct BackedBuffer {
  std::shared_ptr<vk::Buffer>       mBuffer;
  std::shared_ptr<vk::DeviceMemory> mMemory;
};

// -------------------------------------------------------------------------------------------------
class Engine {

 public:
  // -------------------------------------------------------------------------------- public methods
  Engine(std::string const& appName, bool debugMode = true);
  virtual ~Engine();

  // ----------------------------------------------------------------------------- rendering methods

  // void addRenderPass(std::weak_ptr<RenderPass> const& renderPass);
  // void render();

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
    void*                   data = nullptr) const;

  std::shared_ptr<BackedBuffer> createVertexBuffer(vk::DeviceSize size, void* data) const;
  std::shared_ptr<BackedBuffer> createIndexBuffer(vk::DeviceSize size, void* data) const;

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
  std::shared_ptr<vk::SurfaceKHR>          createSurface(GLFWwindow* window) const;
  std::shared_ptr<vk::SwapchainKHR>        createSwapChainKhr(vk::SwapchainCreateInfoKHR const&) const;
  // clang-format on

  // ------------------------------------------------------------------------- vulkan helper methods
  vk::CommandBuffer beginSingleTimeGraphicsCommands() const;
  void              endSingleTimeGraphicsCommands(vk::CommandBuffer commandBuffer) const;

  vk::CommandBuffer beginSingleTimeComputeCommands() const;
  void              endSingleTimeComputeCommands(vk::CommandBuffer commandBuffer) const;

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

  static bool isColorFormat(vk::Format format);
  static bool isDepthFormat(vk::Format format);
  static bool isDepthOnlyFormat(vk::Format format);
  static bool isDepthStencilFormat(vk::Format format);

  // -------------------------------------------------------------------------------- vulkan getters
  std::shared_ptr<vk::Instance> const&   getInstance() const { return mInstance; }
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
  std::shared_ptr<vk::Instance> createInstance(
    std::string const& engine, std::string const& app) const;
  std::shared_ptr<vk::DebugReportCallbackEXT> createDebugCallback() const;
  std::shared_ptr<PhysicalDevice>             createPhysicalDevice() const;
  std::shared_ptr<vk::Device>                 createDevice() const;

  // ------------------------------------------------------------------------------- private members
  bool mDebugMode{false};

  std::shared_ptr<vk::Instance>               mInstance;
  std::shared_ptr<vk::DebugReportCallbackEXT> mDebugCallback;
  std::shared_ptr<PhysicalDevice>             mPhysicalDevice;
  std::shared_ptr<vk::Device>                 mDevice;

  std::shared_ptr<vk::CommandPool> mGraphicsCommandPool;
  std::shared_ptr<vk::CommandPool> mComputeCommandPool;

  vk::Queue mGraphicsQueue, mComputeQueue, mPresentQueue;

  std::vector<std::weak_ptr<RenderPass>> mRenderPasses;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_ENGINE_HPP
