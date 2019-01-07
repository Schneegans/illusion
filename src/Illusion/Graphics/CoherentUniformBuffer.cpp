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
    DevicePtr const& device, vk::DeviceSize size, vk::DeviceSize alignment)
    : mDevice(device)
    , mBuffer(device->createBackedBuffer(vk::BufferUsageFlagBits::eUniformBuffer,
          vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
          size))
    , mAlignment(alignment) {

  mMappedData = (uint8_t*)mDevice->getHandle()->mapMemory(
      *mBuffer->mMemory, 0, mBuffer->mMemoryInfo.allocationSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CoherentUniformBuffer::~CoherentUniformBuffer() {
  mDevice->getHandle()->unmapMemory(*mBuffer->mMemory);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CoherentUniformBuffer::reset() {
  mCurrentWriteOffset = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DeviceSize CoherentUniformBuffer::addData(uint8_t const* data, vk::DeviceSize count) {
  vk::DeviceSize offset = mCurrentWriteOffset;
  updateData(data, count, offset);
  mCurrentWriteOffset += count;

  if (mAlignment > 0) {
    mCurrentWriteOffset += mAlignment - mCurrentWriteOffset % mAlignment;
  }

  return offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CoherentUniformBuffer::updateData(
    uint8_t const* data, vk::DeviceSize count, vk::DeviceSize offset) {

  if (offset + count > mBuffer->mMemoryInfo.allocationSize) {
    throw std::runtime_error("Failed to set uniform data: Preallocated memory exhausted!");
  }

  std::memcpy(mMappedData + offset, data, count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& CoherentUniformBuffer::getBuffer() const {
  return mBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
