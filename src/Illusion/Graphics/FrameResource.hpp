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
// In Illusion, per-frame resources are implemented with two classes: The FrameResourceIndex and  //
// the actual FrameResource. In your application, you will typically have one FrameResourceIndex  //
// and many FrameResources.                                                                       //
// The FrameResourceIndex is used by the FrameResource. The FrameResource template wraps anything //
// you like in a ring-buffer internally. The index into its ring buffer is defined by the         //
// FrameResourceIndex which is passed as first parameter to its constructor.                      //
// There are multiple ways how to use this template. Either you create a struct containing all of //
// your per-frame resources (usually as std::shared_ptr's) and use this as template parameter for //
// a FrameResource. Or you could wrap all individual per-frame resources (each stored in a        //
// shared_ptr) in a FrameResource. Both are valid approaches.                                     //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class FrameResource {
 public:
  // The first argument is the FrameResourceIndex which will be used to index in the internal ring
  // buffer. The second argument is a function (lambda) which is invoked once for each ring buffer
  // entry at construction time, serving as a factory. It should return an instance of the wrapped
  // type. The factory is not kept around, so anything which is captured in the lambda will be
  // released as soon as the constructor returns.
  FrameResource(FrameResourceIndexConstPtr const& index, std::function<T(uint32_t)> const& factory)
      : mRingBuffer(index->indexCount())
      , mIndex(index) {

    for (uint32_t i(0); i < index->indexCount(); ++i) {
      mRingBuffer[i] = factory(i);
    }
  }

  virtual ~FrameResource() = default;

  // Returns a reference to the currently active ring buffer element
  T const& current() const {
    return mRingBuffer[mIndex->current()];
  }
  T& current() {
    return mRingBuffer[mIndex->current()];
  }

  // Returns a reference to the ring buffer element which will be active once mIndex->step() has
  // been called once more.
  T const& next() const {
    return mRingBuffer[mIndex->next()];
  }
  T& next() {
    return mRingBuffer[mIndex->next()];
  }

  // Returns a reference to the ring buffer element which was active before mIndex->step() has been
  // called the las time.
  T const& previous() const {
    return mRingBuffer[mIndex->previous()];
  }
  T& previous() {
    return mRingBuffer[mIndex->previous()];
  }

  // Returns the number of ring buffer elements. Can be used in conjunction with the operators below
  // in order to iterate all ring buffer elements.
  uint32_t size() const {
    return mIndex->indexCount();
  }
  T const& operator[](size_t i) const {
    return mRingBuffer[i];
  }
  T& operator[](size_t i) {
    return mRingBuffer[i];
  }

  // These iterators can be used to iterate all ring buffer elements in range-based for-loops for
  // example.
  typename std::vector<T>::iterator begin() {
    return mRingBuffer.begin();
  }

  typename std::vector<T>::iterator end() {
    return mRingBuffer.end();
  }

  typename std::vector<T>::const_iterator begin() const {
    return mRingBuffer.begin();
  }

  typename std::vector<T>::const_iterator end() const {
    return mRingBuffer.end();
  }

 private:
  std::vector<T>             mRingBuffer;
  FrameResourceIndexConstPtr mIndex;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_FRAME_RESOURCE_HPP
