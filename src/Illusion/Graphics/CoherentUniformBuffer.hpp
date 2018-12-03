////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_COHERENT_UNIFORM_BUFFER_HPP
#define ILLUSION_GRAPHICS_COHERENT_UNIFORM_BUFFER_HPP

#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CoherentUniformBuffer {
 public:
  CoherentUniformBuffer(std::shared_ptr<Device> const& device, vk::DeviceSize size);
  virtual ~CoherentUniformBuffer();

  vk::DeviceSize addData(uint8_t const* data, vk::DeviceSize count);

  template <typename T>
  vk::DeviceSize addData(T const& data) {
    addData((uint8_t*)&data, sizeof(data));
  }

  vk::DeviceSize updateData(uint8_t const* data, vk::DeviceSize count, vk::DeviceSize offset);

  template <typename T>
  void updateData(T const& data, vk::DeviceSize offset = 0) {
    updateData((uint8_t*)&data, sizeof(data), offset);
  }

  std::shared_ptr<BackedBuffer> const& getBuffer() const;

 private:
  std::shared_ptr<Device>       mDevice;
  std::shared_ptr<BackedBuffer> mBuffer;
  uint8_t*                      mMappedData         = nullptr;
  vk::DeviceSize                mCurrentWriteOffset = 0;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_COHERENT_UNIFORM_BUFFER_HPP
