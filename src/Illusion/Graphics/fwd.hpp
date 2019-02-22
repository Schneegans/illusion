////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
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

class CoherentBuffer;
class CommandBuffer;
class DescriptorPool;
class DescriptorSetReflection;
class Device;
class FrameResourceIndex;
class GlslShader;
class Instance;
class PhysicalDevice;
class PipelineReflection;
class RenderPass;
class LazyRenderPass;
class Shader;
class ShaderModule;
class ShaderSource;
class Swapchain;
class Window;

typedef std::shared_ptr<BackedBuffer>       BackedBufferPtr;
typedef std::shared_ptr<const BackedBuffer> BackedBufferConstPtr;
typedef std::shared_ptr<BackedImage>        BackedImagePtr;
typedef std::shared_ptr<const BackedImage>  BackedImageConstPtr;
typedef std::shared_ptr<Texture>            TexturePtr;
typedef std::shared_ptr<const Texture>      TextureConstPtr;

typedef std::shared_ptr<CoherentBuffer>                CoherentBufferPtr;
typedef std::shared_ptr<const CoherentBuffer>          CoherentBufferConstPtr;
typedef std::shared_ptr<CommandBuffer>                 CommandBufferPtr;
typedef std::shared_ptr<const CommandBuffer>           CommandBufferConstPtr;
typedef std::shared_ptr<DescriptorPool>                DescriptorPoolPtr;
typedef std::shared_ptr<const DescriptorPool>          DescriptorPoolConstPtr;
typedef std::shared_ptr<DescriptorSetReflection>       DescriptorSetReflectionPtr;
typedef std::shared_ptr<const DescriptorSetReflection> DescriptorSetReflectionConstPtr;
typedef std::shared_ptr<Device>                        DevicePtr;
typedef std::shared_ptr<const Device>                  DeviceConstPtr;
typedef std::shared_ptr<FrameResourceIndex>            FrameResourceIndexPtr;
typedef std::shared_ptr<const FrameResourceIndex>      FrameResourceIndexConstPtr;
typedef std::shared_ptr<GlslShader>                    GlslShaderPtr;
typedef std::shared_ptr<const GlslShader>              GlslShaderConstPtr;
typedef std::shared_ptr<Instance>                      InstancePtr;
typedef std::shared_ptr<const Instance>                InstanceConstPtr;
typedef std::shared_ptr<PhysicalDevice>                PhysicalDevicePtr;
typedef std::shared_ptr<const PhysicalDevice>          PhysicalDeviceConstPtr;
typedef std::shared_ptr<PipelineReflection>            PipelineReflectionPtr;
typedef std::shared_ptr<const PipelineReflection>      PipelineReflectionConstPtr;
typedef std::shared_ptr<RenderPass>                    RenderPassPtr;
typedef std::shared_ptr<const RenderPass>              RenderPassConstPtr;
typedef std::shared_ptr<LazyRenderPass>                LazyRenderPassPtr;
typedef std::shared_ptr<const LazyRenderPass>          LazyRenderPassConstPtr;
typedef std::shared_ptr<Shader>                        ShaderPtr;
typedef std::shared_ptr<const Shader>                  ShaderConstPtr;
typedef std::shared_ptr<ShaderModule>                  ShaderModulePtr;
typedef std::shared_ptr<const ShaderModule>            ShaderModuleConstPtr;
typedef std::shared_ptr<ShaderSource>                  ShaderSourcePtr;
typedef std::shared_ptr<const ShaderSource>            ShaderSourceConstPtr;
typedef std::shared_ptr<Swapchain>                     SwapchainPtr;
typedef std::shared_ptr<const Swapchain>               SwapchainConstPtr;
typedef std::shared_ptr<Window>                        WindowPtr;
typedef std::shared_ptr<const Window>                  WindowConstPtr;

namespace Gltf {
class Model;
struct Animation;
struct Material;
struct Mesh;
struct Node;
struct Skin;

typedef std::shared_ptr<Animation>       AnimationPtr;
typedef std::shared_ptr<const Animation> AnimationConstPtr;
typedef std::shared_ptr<Material>        MaterialPtr;
typedef std::shared_ptr<const Material>  MaterialConstPtr;
typedef std::shared_ptr<Mesh>            MeshPtr;
typedef std::shared_ptr<const Mesh>      MeshConstPtr;
typedef std::shared_ptr<Model>           ModelPtr;
typedef std::shared_ptr<const Model>     ModelConstPtr;
typedef std::shared_ptr<Node>            NodePtr;
typedef std::shared_ptr<const Node>      NodeConstPtr;
typedef std::shared_ptr<Skin>            SkinPtr;
typedef std::shared_ptr<const Skin>      SkinConstPtr;
} // namespace Gltf

} // namespace Illusion::Graphics

namespace vk {

typedef std::shared_ptr<const vk::Buffer>                 BufferPtr;
typedef std::shared_ptr<const vk::CommandBuffer>          CommandBufferPtr;
typedef std::shared_ptr<const vk::CommandPool>            CommandPoolPtr;
typedef std::shared_ptr<const vk::DebugUtilsMessengerEXT> DebugUtilsMessengerEXTPtr;
typedef std::shared_ptr<const vk::DescriptorPool>         DescriptorPoolPtr;
typedef std::shared_ptr<const vk::DescriptorSet>          DescriptorSetPtr;
typedef std::shared_ptr<const vk::DescriptorSetLayout>    DescriptorSetLayoutPtr;
typedef std::shared_ptr<const vk::Device>                 DevicePtr;
typedef std::shared_ptr<const vk::DeviceMemory>           DeviceMemoryPtr;
typedef std::shared_ptr<const vk::Fence>                  FencePtr;
typedef std::shared_ptr<const vk::Framebuffer>            FramebufferPtr;
typedef std::shared_ptr<const vk::Image>                  ImagePtr;
typedef std::shared_ptr<const vk::ImageView>              ImageViewPtr;
typedef std::shared_ptr<const vk::Instance>               InstancePtr;
typedef std::shared_ptr<const vk::Pipeline>               PipelinePtr;
typedef std::shared_ptr<const vk::PipelineLayout>         PipelineLayoutPtr;
typedef std::shared_ptr<const vk::RenderPass>             RenderPassPtr;
typedef std::shared_ptr<const vk::Sampler>                SamplerPtr;
typedef std::shared_ptr<const vk::Semaphore>              SemaphorePtr;
typedef std::shared_ptr<const vk::ShaderModule>           ShaderModulePtr;
typedef std::shared_ptr<const vk::SurfaceKHR>             SurfaceKHRPtr;
typedef std::shared_ptr<const vk::SwapchainKHR>           SwapchainKHRPtr;

} // namespace vk

#endif // ILLUSION_GRAPHICS_FWD_HPP
