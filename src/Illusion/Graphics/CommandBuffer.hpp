////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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
////////////////////////////////////////////////////////////////////////////////////////////////////

class CommandBuffer {
 public:
  template <typename... Args>
  static CommandBufferPtr create(Args&&... args) {
    return std::make_shared<CommandBuffer>(args...);
  };

  CommandBuffer(DevicePtr const& device, QueueType type = QueueType::eGeneric,
    vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

  void reset();
  void begin(
    vk::CommandBufferUsageFlagBits usage = vk::CommandBufferUsageFlagBits::eSimultaneousUse) const;
  void end() const;
  void submit(std::vector<vk::Semaphore> const& waitSemaphores   = {},
    std::vector<vk::PipelineStageFlags> const&  waitStages       = {},
    std::vector<vk::Semaphore> const&           signalSemaphores = {},
    vk::Fence const&                            fence            = nullptr) const;

  void waitIdle() const;

  void beginRenderPass(RenderPassPtr const& renderPass);
  void endRenderPass();

  void bindIndexBuffer(
    BackedBufferPtr const& buffer, vk::DeviceSize offset, vk::IndexType indexType) const;

  void bindVertexBuffers(uint32_t                                  firstBinding,
    std::vector<std::pair<BackedBufferPtr, vk::DeviceSize>> const& buffersAndOffsets) const;

  void bindVertexBuffers(uint32_t firstBinding, std::vector<BackedBufferPtr> const& buffers) const;

  void bindCombinedImageSampler(TexturePtr const& texture, uint32_t set, uint32_t binding) const;

  void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0,
    uint32_t firstInstance = 0);

  void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0,
    int32_t vertexOffset = 0, uint32_t firstInstance = 0);

  void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

  void                    setShaderProgram(ShaderProgramPtr const& val);
  ShaderProgramPtr const& getShaderProgram() const;

  GraphicsState& graphicsState();
  BindingState&  bindingState();

  void pushConstants(const void* data, uint32_t size, uint32_t offset = 0) const;

  template <typename T>
  void pushConstants(T const& data, uint32_t offset = 0) const {
    pushConstants(&data, sizeof(T), offset);
  }

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
  void flush();

  vk::PipelinePtr getPipelineHandle();

  DevicePtr              mDevice;
  vk::CommandBufferPtr   mVkCmd;
  QueueType              mType;
  vk::CommandBufferLevel mLevel;

  GraphicsState      mGraphicsState;
  BindingState       mBindingState;
  DescriptorSetCache mDescriptorSetCache;

  ShaderProgramPtr                  mCurrentShaderProgram;
  RenderPassPtr                     mCurrentRenderPass;
  uint32_t                          mCurrentSubPass = 0;
  std::map<uint32_t, Core::BitHash> mCurrentDescriptorSetLayoutHashes;

  std::map<Core::BitHash, vk::PipelinePtr> mPipelineCache;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_COMMAND_BUFFER_HPP
