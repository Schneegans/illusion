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

#include "fwd.hpp"

#include <glm/glm.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CommandBuffer : public vk::CommandBuffer {
 public:
  CommandBuffer(vk::CommandBuffer const& base);

  template <typename T>
  void pushConstants(
    vk::PipelineLayout const& layout,
    vk::ShaderStageFlags      stages,
    T const&                  data,
    uint32_t                  offset = 0) {

    vk::CommandBuffer::pushConstants(layout, stages, offset, sizeof(T), &data);
  }

  void transitionImageLayout(
    vk::Image                 image,
    vk::ImageLayout           oldLayout,
    vk::ImageLayout           newLayout,
    vk::PipelineStageFlagBits stage = vk::PipelineStageFlagBits::eTopOfPipe,
    vk::ImageSubresourceRange range = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}) const;

  void copyImage(vk::Image src, vk::Image dst, glm::uvec2 const& size) const;
  void blitImage(
    vk::Image         src,
    vk::Image         dst,
    glm::uvec2 const& srcSize,
    glm::uvec2 const& dstSize,
    vk::Filter        filter) const;
  void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const;

 private:
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_COMMAND_BUFFER_HPP
