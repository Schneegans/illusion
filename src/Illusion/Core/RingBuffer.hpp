////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_RING_BUFFER_HPP
#define ILLUSION_CORE_RING_BUFFER_HPP

#include <array>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, int ringBufferSize>
class RingBuffer {

 public:
  template <typename... Args>
  explicit RingBuffer(Args&&... a)
      : mBuffer{a...} {
  }

  virtual ~RingBuffer() = default;

  T& next() {
    mIndex = (mIndex + 1) % ringBufferSize;
    return current();
  }

  T& current() {
    return mBuffer[mIndex];
  }

 protected:
  std::array<T, ringBufferSize> mBuffer;
  uint32_t                      mIndex = 0;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_RING_BUFFER_HPP
