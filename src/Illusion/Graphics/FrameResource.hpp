////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_FRAME_RESOURCE_HPP
#define ILLUSION_GRAPHICS_FRAME_RESOURCE_HPP

#include "FrameResourceIndex.hpp"

#include <functional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class FrameResource {

 public:
  FrameResource(FrameResourceIndexPtr const& index, std::function<T()> const& factory)
      : mBuffer(index->count())
      , mIndex(index) {

    for (auto& i : mBuffer) {
      i = factory();
    }
  }

  virtual ~FrameResource() = default;

  T const& current() const {
    return mBuffer[mIndex->current()];
  }

  T& current() {
    return mBuffer[mIndex->current()];
  }

  T const& next() const {
    return mBuffer[mIndex->next()];
  }

  T& next() {
    return mBuffer[mIndex->next()];
  }

  T const& previous() const {
    return mBuffer[mIndex->previous()];
  }

  T& previous() {
    return mBuffer[mIndex->previous()];
  }

  uint32_t count() const {
    return mIndex->count();
  }

  typename std::vector<T>::iterator begin() {
    return mBuffer.begin();
  }

  typename std::vector<T>::iterator end() {
    return mBuffer.end();
  }

  typename std::vector<T>::const_iterator begin() const {
    return mBuffer.begin();
  }

  typename std::vector<T>::const_iterator end() const {
    return mBuffer.end();
  }

 protected:
  std::vector<T>        mBuffer;
  FrameResourceIndexPtr mIndex;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_FRAME_RESOURCE_HPP
