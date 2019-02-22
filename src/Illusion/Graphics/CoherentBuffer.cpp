////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CoherentBuffer.hpp"

#include "BackedBuffer.hpp"
#include "Device.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

CoherentBuffer::CoherentBuffer(std::string const& name, DeviceConstPtr const& device,
    vk::DeviceSize size, vk::BufferUsageFlagBits usage, vk::DeviceSize alignment)
    : Core::NamedObject(name)
    , mDevice(device)
    , mBuffer(device->createBackedBuffer(name, usage,
          vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
          size))
    , mAlignment(alignment) {

  // Map the data. It will stay mapped until this object is deleted.
  mMappedData = reinterpret_cast<uint8_t*>(
      mDevice->getHandle()->mapMemory(*mBuffer->mMemory, 0, mBuffer->mMemoryInfo.allocationSize));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CoherentBuffer::~CoherentBuffer() {
  // Unmap the data again.
  mDevice->getHandle()->unmapMemory(*mBuffer->mMemory);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CoherentBuffer::reset() {
  mCurrentWriteOffset = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DeviceSize CoherentBuffer::addData(uint8_t const* data, vk::DeviceSize count) {
  vk::DeviceSize offset = mCurrentWriteOffset;
  updateData(data, count, offset);
  mCurrentWriteOffset += count;

  // Add alignment based padding.
  if (mAlignment > 0) {
    mCurrentWriteOffset += mAlignment - mCurrentWriteOffset % mAlignment;
  }

  return offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CoherentBuffer::updateData(uint8_t const* data, vk::DeviceSize count, vk::DeviceSize offset) {

  if (offset + count > mBuffer->mMemoryInfo.allocationSize) {
    throw std::runtime_error("Failed to set uniform data: Preallocated memory exhausted!");
  }

  std::memcpy(mMappedData + offset, data, count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& CoherentBuffer::getBuffer() const {
  return mBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
