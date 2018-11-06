////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_UNIFORM_BUFFER_HPP
#define ILLUSION_GRAPHICS_UNIFORM_BUFFER_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Window.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
template <typename T>
class UniformBuffer : public T {

 public:
  // -------------------------------------------------------------------------------- public methods
  UniformBuffer(std::shared_ptr<Engine> const& engine)
    : mEngine(engine) {

    mBuffer = engine->createBackedBuffer(
      sizeof(T),
      vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal);
  }

  void update(vk::CommandBuffer const& cmd) const {
    cmd.updateBuffer(*mBuffer->mBuffer, 0, sizeof(T), (uint8_t*)this);
  }

  void bind(vk::DescriptorSet const& descriptorSet) const {
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = *mBuffer->mBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range  = sizeof(T);

    vk::WriteDescriptorSet info;
    info.dstSet          = descriptorSet;
    info.dstBinding      = T::getBindingPoint();
    info.dstArrayElement = 0;
    info.descriptorType  = vk::DescriptorType::eUniformBuffer;
    info.descriptorCount = 1;
    info.pBufferInfo     = &bufferInfo;

    mEngine->getDevice()->updateDescriptorSets(info, nullptr);
  }

 private:
  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Engine>       mEngine;
  std::shared_ptr<BackedBuffer> mBuffer;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_UNIFORM_BUFFER_HPP
