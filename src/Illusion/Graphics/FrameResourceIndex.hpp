////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_FRAME_RESOURCE_INDEX_HPP
#define ILLUSION_GRAPHICS_FRAME_RESOURCE_INDEX_HPP

#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// In Illusion, per-frame resources are implemented with two classes: The FrameResourceIndex and  //
// the actual FrameResource. In your application, you will typically have one FrameResourceIndex  //
// and many FrameResources.                                                                       //
// The FrameResourceIndex keeps track of an index (a simple uint32_t) in  a looped fashion. That  //
// means it can be increased with its step() method, but it will be reset to zero once its        //
// allowed maximum is reached.                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////

class FrameResourceIndex : public Core::StaticCreate<FrameResourceIndex> {
 public:
  // The parameter determines how many different indices can be returned by this instance. An
  // indexCount of 2 means that the current index will alternate between 0 and 1.
  FrameResourceIndex(uint32_t indexCount = 2);

  // Calculates mIndex = (mIndex + 1) % mIndexCount.
  void step();

  // Returns the current index.
  uint32_t current() const;

  // Returns the index which will be current once step() has been called once more.
  uint32_t next() const;

  // Returns the index which was current before step() has been called the last time.
  uint32_t previous() const;

  // Returns the number of different indices which can be returned by this instance. This is the
  // number which was given at construction time.
  uint32_t indexCount() const;

 protected:
  uint32_t       mIndex      = 0;
  const uint32_t mIndexCount = 2;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_FRAME_RESOURCE_INDEX_HPP
