////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_FRAME_RESOURCE_INDEX_HPP
#define ILLUSION_GRAPHICS_FRAME_RESOURCE_INDEX_HPP

#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class FrameResourceIndex {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static FrameResourceIndexPtr create(Args&&... args) {
    return std::make_shared<FrameResourceIndex>(args...);
  };

  FrameResourceIndex(uint32_t indexCount = 2);

  void step();

  uint32_t current() const;
  uint32_t next() const;
  uint32_t previous() const;

  uint32_t count() const;

 protected:
  uint32_t       mIndex      = 0;
  const uint32_t mIndexCount = 2;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_FRAME_RESOURCE_INDEX_HPP
