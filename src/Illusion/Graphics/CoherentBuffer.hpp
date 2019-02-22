////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_COHERENT_BUFFER_HPP
#define ILLUSION_GRAPHICS_COHERENT_BUFFER_HPP

#include "../Core/NamedObject.hpp"
#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This class can be used to manage a block of coherently mapped video memory. Typical use case   //
// include frequently updated uniform buffers (usage = vk::BufferUsageFlagBits::eUniformBuffer)   //
// or frequently updated storage buffers (usage = vk::BufferUsageFlagBits::eStorageBuffer). There //
// is no mechanism to ensure that the data is not currently read by the GPU. You have to          //
// extrnally synchronize access somehow.                                                          //
////////////////////////////////////////////////////////////////////////////////////////////////////

class CoherentBuffer : public Core::StaticCreate<CoherentBuffer>, public Core::NamedObject {
 public:
  // The alignment value is used by the addData() method to add some spacing between adjacent memory
  // blocks which may be required when used for dynamic uniform or storage buffers. You can use
  // instance->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment to query
  // the required value of your implementation.
  CoherentBuffer(std::string const& name, DeviceConstPtr const& device, vk::DeviceSize size,
      vk::BufferUsageFlagBits usage, vk::DeviceSize alignment = 0);
  virtual ~CoherentBuffer();

  // Resets the current write offset (not the actual data of the buffer). Preceding calls to
  // addData() will start to write data to the beginning of the buffer again.
  void reset();

  // Writes the given data to the buffer. Afterwards the current write offset is increased by
  // "count" so that preceding calls to addData will append the data. If an alginment was specified
  // at construction time, padding may be added to the write-offset as required.
  vk::DeviceSize addData(uint8_t const* data, vk::DeviceSize count);

  // Convenience method for built-ins or simple structs which calls the method above.
  template <typename T>
  vk::DeviceSize addData(T const& data) {
    return addData((uint8_t*)&data, sizeof(data));
  }

  // Directly writes data to the given offset. The current write offset and the alignment specified
  // at construction time are ignored.
  void updateData(uint8_t const* data, vk::DeviceSize count, vk::DeviceSize offset);

  // Convenience method for built-ins or simple structs which calls the method above.
  template <typename T>
  void updateData(T const& data, vk::DeviceSize offset = 0) {
    updateData((uint8_t*)&data, sizeof(data), offset);
  }

  // Access to the internal buffer.
  BackedBufferPtr const& getBuffer() const;

 private:
  DeviceConstPtr  mDevice;
  BackedBufferPtr mBuffer;
  uint8_t*        mMappedData         = nullptr;
  vk::DeviceSize  mCurrentWriteOffset = 0;
  vk::DeviceSize  mAlignment          = 0;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_COHERENT_BUFFER_HPP
