////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_COMMAND_BUFFER_HPP
#define ILLUSION_GRAPHICS_COMMAND_BUFFER_HPP

#include "BindingState.hpp"
#include "DescriptorSetCache.hpp"
#include "GraphicsState.hpp"
#include "fwd.hpp"

#include <glm/glm.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The CommandBuffer class encapsulates a vk::CommandBuffer. It tracks the bound shader program,  //
// the current render-pass and sub-pass, the graphics and the binding state. This information is  //
// used to create descriptor sets and pipelines on-the-fly. Both are cached and re-used when      //
// possible.                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////

class CommandBuffer : public Core::StaticCreate<CommandBuffer>, public Core::NamedObject {
 public:
  // Allocates a new vk::CommandBuffer from the device. It is a good idea to give the object a
  // descriptive name.
  CommandBuffer(std::string const& name, DevicePtr const& device,
      QueueType              type  = QueueType::eGeneric,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

  // basic operations ------------------------------------------------------------------------------

  // Resets the vk::CommandBuffer and clears the current binding state.
  // The current graphics state and the current shader program are not changed.
  void reset();

  // Begins the internal vk::CommandBuffer.
  void begin(vk::CommandBufferUsageFlagBits usage =
                 vk::CommandBufferUsageFlagBits::eSimultaneousUse) const;

  // Ends the internal vk::CommandBuffer.
  void end() const;

  // Submits the internal vk::CommandBuffer to the Device's queue matching the QueueType given to
  // this CommandBuffer at construction time.
  void submit(std::vector<vk::SemaphorePtr> const& waitSemaphores   = {},
      std::vector<vk::PipelineStageFlags> const&   waitStages       = {},
      std::vector<vk::SemaphorePtr> const&         signalSemaphores = {},
      vk::FencePtr const&                          fence            = nullptr) const;

  // Calls waitIdle() on the Device's queue matching the QueueType given to this CommandBuffer at
  // construction time.
  void waitIdle() const;

  // Stores and begins the given RenderPass.
  void beginRenderPass(RenderPassPtr const& renderPass);

  // Ends and releases the current RenderPass.
  void endRenderPass();

  // state modification ----------------------------------------------------------------------------

  // Read and write access to the current GraphicsState. Changes will not directly affect the
  // internal vk::CommandBuffer; they are flushed whenever a draw or dispatch command is issued.
  GraphicsState&       graphicsState();
  GraphicsState const& graphicsState() const;

  // Read and write access to the current BindingState. Changes will not directly affect the
  // internal vk::CommandBuffer; they are flushed whenever a draw or dispatch command is issued.
  BindingState&       bindingState();
  BindingState const& bindingState() const;

  // Read and write access to the currently bound Shader. Changes will not directly affect
  // the internal vk::CommandBuffer; they are flushed whenever a draw or dispatch command is issued.
  void             setShader(ShaderPtr const& val);
  ShaderPtr const& getShader() const;

  // Binds the given BackedBuffer as index buffer. This is directly recorded to the internal
  // vk::CommandBuffer.
  void bindIndexBuffer(
      BackedBufferPtr const& buffer, vk::DeviceSize offset, vk::IndexType indexType) const;

  // Binds the given BackedBuffer as vertex buffer. Compared to the method below, this method allows
  // for additional offsets. This is directly recorded to the internal vk::CommandBuffer.
  void bindVertexBuffers(uint32_t                                    firstBinding,
      std::vector<std::pair<BackedBufferPtr, vk::DeviceSize>> const& buffersAndOffsets) const;

  // Binds the given BackedBuffer as vertex buffer. This is directly recorded to the internal
  // vk::CommandBuffer.
  void bindVertexBuffers(uint32_t firstBinding, std::vector<BackedBufferPtr> const& buffers) const;

  // Sets the given data (size in bytes) as push constant data. You have to make sure that there is
  // a shader program currently bound. This will throw a std::runtime_error when there is no active
  // shader or when the active shader does not define a push constant buffer.
  void pushConstants(const void* data, uint32_t size, uint32_t offset = 0) const;

  // Convenience method calling the method above. This allows setting any type (e.g. structs) as
  // push constant data. This will throw a std::runtime_error when there is no active shader or when
  // the active shader does not define a push constant buffer.
  template <typename T>
  void pushConstants(T const& data, uint32_t offset = 0) const {
    pushConstants(&data, sizeof(T), offset);
  }

  // draw calls ------------------------------------------------------------------------------------

  // These draw and dispatch calls are directly recorded to the internal vk::CommandBuffer. Before,
  // the private method flush() is called. This will either create a vk::Pipeline object based on
  // the currently bound shader program and the current graphics state or retrieve a matching cached
  // vk::Pipeline. This pipeline will be bound. Then, based on the binding state, descriptor sets
  // will be allocated, updated and bound. These will throw a std::runtime_error when there is no
  // active shader.
  void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0,
      uint32_t firstInstance = 0);
  void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0,
      int32_t vertexOffset = 0, uint32_t firstInstance = 0);
  void dispatch(uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);

  // convenience methods ---------------------------------------------------------------------------

  // This will throw a std::runtime_error when the layout transition is impossible.
  void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
      vk::PipelineStageFlagBits srcStage, vk::PipelineStageFlagBits dstStage,
      vk::ImageSubresourceRange range = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}) const;

  void copyImage(vk::Image src, vk::Image dst, glm::uvec2 const& size) const;

  void blitImage(vk::Image src, vk::Image dst, glm::uvec2 const& srcSize, glm::uvec2 const& dstSize,
      vk::Filter filter) const;

  void blitImage(vk::Image src, uint32_t srcMipmapLevel, vk::Image dst, uint32_t dstMipmapLevel,
      glm::uvec2 const& srcSize, glm::uvec2 const& dstSize, uint32_t layerCount,
      vk::Filter filter) const;

  void resolveImage(vk::Image src, vk::ImageLayout srcLayout, vk::Image dst,
      vk::ImageLayout dstLayout, vk::ImageResolve region) const;

  void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const;

  void copyBufferToImage(vk::Buffer src, vk::Image dst, vk::ImageLayout dstLayout,
      std::vector<vk::BufferImageCopy> const& infos) const;

 private:
  void            flush();
  vk::PipelinePtr getPipelineHandle();

  DevicePtr              mDevice;
  vk::CommandBufferPtr   mVkCmd;
  QueueType              mType;
  vk::CommandBufferLevel mLevel;

  GraphicsState mGraphicsState;
  BindingState  mBindingState;

  ShaderPtr     mCurrentShader;
  RenderPassPtr mCurrentRenderPass;
  uint32_t      mCurrentSubPass = 0;

  std::map<Core::BitHash, vk::PipelinePtr> mPipelineCache;

  struct DescriptorSetState {
    vk::DescriptorSetPtr mSet;
    Core::BitHash        mSetLayoutHash;
  };
  std::map<uint32_t, DescriptorSetState> mCurrentDescriptorSets;
  DescriptorSetCache                     mDescriptorSetCache;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_COMMAND_BUFFER_HPP
