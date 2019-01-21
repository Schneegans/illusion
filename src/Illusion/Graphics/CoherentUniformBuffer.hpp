////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_COHERENT_UNIFORM_BUFFER_HPP
#define ILLUSION_GRAPHICS_COHERENT_UNIFORM_BUFFER_HPP

#include "../Core/NamedObject.hpp"
#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CoherentUniformBuffer : public Core::StaticCreate<CoherentUniformBuffer>,
                              public Core::NamedObject {
 public:
  CoherentUniformBuffer(std::string const& name, DevicePtr const& device, vk::DeviceSize size,
      vk::DeviceSize alignment = 0);
  virtual ~CoherentUniformBuffer();

  void reset();

  vk::DeviceSize addData(uint8_t const* data, vk::DeviceSize count);

  template <typename T>
  vk::DeviceSize addData(T const& data) {
    return addData((uint8_t*)&data, sizeof(data));
  }

  void updateData(uint8_t const* data, vk::DeviceSize count, vk::DeviceSize offset);

  template <typename T>
  void updateData(T const& data, vk::DeviceSize offset = 0) {
    updateData((uint8_t*)&data, sizeof(data), offset);
  }

  BackedBufferPtr const& getBuffer() const;

 private:
  DevicePtr       mDevice;
  BackedBufferPtr mBuffer;
  uint8_t*        mMappedData         = nullptr;
  vk::DeviceSize  mCurrentWriteOffset = 0;
  vk::DeviceSize  mAlignment          = 0;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_COHERENT_UNIFORM_BUFFER_HPP
