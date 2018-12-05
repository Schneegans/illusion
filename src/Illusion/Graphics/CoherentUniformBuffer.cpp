////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CoherentUniformBuffer.hpp"

#include "Device.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

CoherentUniformBuffer::CoherentUniformBuffer(
  std::shared_ptr<Device> const& device, vk::DeviceSize size)
  : mDevice(device)
  , mBuffer(device->createBackedBuffer(
      size,
      vk::BufferUsageFlagBits::eUniformBuffer,
      vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)) {

  mMappedData = (uint8_t*)mDevice->getHandle()->mapMemory(*mBuffer->mMemory, 0, mBuffer->mSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CoherentUniformBuffer::~CoherentUniformBuffer() {
  mDevice->getHandle()->unmapMemory(*mBuffer->mMemory);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DeviceSize CoherentUniformBuffer::addData(uint8_t const* data, vk::DeviceSize count) {
  vk::DeviceSize offset = mCurrentWriteOffset;
  updateData(data, count, offset);
  mCurrentWriteOffset += count;
  return offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DeviceSize CoherentUniformBuffer::updateData(
  uint8_t const* data, vk::DeviceSize count, vk::DeviceSize offset) {

  if (offset + count > mBuffer->mSize) {
    throw std::runtime_error("Failed to set uniform data: Preallocated memory exhausted!");
  }

  std::memcpy(mMappedData + offset, data, count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackedBuffer> const& CoherentUniformBuffer::getBuffer() const { return mBuffer; }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
