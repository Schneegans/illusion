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

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
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

 private:
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_COMMAND_BUFFER_HPP
