////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_RING_BUFFER_HPP
#define ILLUSION_CORE_RING_BUFFER_HPP

#include <array>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A simple wrapper for a std::array acting as a ring buffer. The arguments passed to the         //
// constructor are directly forwarded to the internal std::array. Use the members next() and      //
// current() to access the ring buffer elements.                                                  //
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
