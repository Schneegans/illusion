////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_BACKED_BUFFER_HPP
#define ILLUSION_GRAPHICS_BACKED_BUFFER_HPP

#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A BackedBuffer stores a vk::Buffer and some vk::DeviceMemory attached to this buffer.          //
// Additionally all create-info objects are stored in order to access the properties of the       //
// buffer and the memory. Use the Device class to easily create a BackedBuffer.                   //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedBuffer {
  vk::BufferPtr       mBuffer;
  vk::DeviceMemoryPtr mMemory;

  vk::BufferCreateInfo   mBufferInfo;
  vk::MemoryAllocateInfo mMemoryInfo;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BACKED_BUFFER_HPP
