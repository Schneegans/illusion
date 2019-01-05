////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_FWD_HPP
#define ILLUSION_GRAPHICS_FWD_HPP

#include <vulkan/vulkan.hpp>

namespace Illusion::Graphics {

enum class QueueType { eGeneric = 0, eCompute = 1, eTransfer = 2 };

struct BackedBuffer;
struct BackedImage;
struct Texture;

class CoherentUniformBuffer;
class CommandBuffer;
class DescriptorPool;
class DescriptorSetReflection;
class Device;
class Engine;
class Framebuffer;
class GltfModel;
class PhysicalDevice;
class PipelineReflection;
class RenderPass;
class ShaderModule;
class Shader;
class GlslShader;
class Swapchain;
class Window;

typedef std::shared_ptr<BackedBuffer> BackedBufferPtr;
typedef std::shared_ptr<BackedImage>  BackedImagePtr;

typedef std::shared_ptr<CoherentUniformBuffer>   CoherentUniformBufferPtr;
typedef std::shared_ptr<CommandBuffer>           CommandBufferPtr;
typedef std::shared_ptr<DescriptorPool>          DescriptorPoolPtr;
typedef std::shared_ptr<DescriptorSetReflection> DescriptorSetReflectionPtr;
typedef std::shared_ptr<Device>                  DevicePtr;
typedef std::shared_ptr<Engine>                  EnginePtr;
typedef std::shared_ptr<Framebuffer>             FramebufferPtr;
typedef std::shared_ptr<GltfModel>               GltfModelPtr;
typedef std::shared_ptr<PhysicalDevice>          PhysicalDevicePtr;
typedef std::shared_ptr<PipelineReflection>      PipelineReflectionPtr;
typedef std::shared_ptr<RenderPass>              RenderPassPtr;
typedef std::shared_ptr<ShaderModule>            ShaderModulePtr;
typedef std::shared_ptr<Shader>                  ShaderPtr;
typedef std::shared_ptr<GlslShader>              GlslShaderPtr;
typedef std::shared_ptr<Swapchain>               SwapchainPtr;
typedef std::shared_ptr<Texture>                 TexturePtr;
typedef std::shared_ptr<Window>                  WindowPtr;

} // namespace Illusion::Graphics

namespace vk {

typedef std::shared_ptr<vk::Buffer>                 BufferPtr;
typedef std::shared_ptr<vk::CommandBuffer>          CommandBufferPtr;
typedef std::shared_ptr<vk::CommandPool>            CommandPoolPtr;
typedef std::shared_ptr<vk::DebugReportCallbackEXT> DebugReportCallbackEXTPtr;
typedef std::shared_ptr<vk::DescriptorPool>         DescriptorPoolPtr;
typedef std::shared_ptr<vk::DescriptorSet>          DescriptorSetPtr;
typedef std::shared_ptr<vk::DescriptorSetLayout>    DescriptorSetLayoutPtr;
typedef std::shared_ptr<vk::Device>                 DevicePtr;
typedef std::shared_ptr<vk::DeviceMemory>           DeviceMemoryPtr;
typedef std::shared_ptr<vk::Fence>                  FencePtr;
typedef std::shared_ptr<vk::Framebuffer>            FramebufferPtr;
typedef std::shared_ptr<vk::Image>                  ImagePtr;
typedef std::shared_ptr<vk::ImageView>              ImageViewPtr;
typedef std::shared_ptr<vk::Instance>               InstancePtr;
typedef std::shared_ptr<vk::Pipeline>               PipelinePtr;
typedef std::shared_ptr<vk::PipelineLayout>         PipelineLayoutPtr;
typedef std::shared_ptr<vk::RenderPass>             RenderPassPtr;
typedef std::shared_ptr<vk::Sampler>                SamplerPtr;
typedef std::shared_ptr<vk::Semaphore>              SemaphorePtr;
typedef std::shared_ptr<vk::ShaderModule>           ShaderModulePtr;
typedef std::shared_ptr<vk::SurfaceKHR>             SurfaceKHRPtr;
typedef std::shared_ptr<vk::SwapchainKHR>           SwapchainKHRPtr;

} // namespace vk

#endif // ILLUSION_GRAPHICS_FWD_HPP
