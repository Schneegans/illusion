////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_INPUT_MOUSE_EVENT_HPP
#define ILLUSION_INPUT_MOUSE_EVENT_HPP

#include "Enums.hpp"

namespace Illusion::Input {

struct MouseEvent {

  enum class Type { eMove, eScroll, ePress, eRelease, eLeave };

  MouseEvent();
  MouseEvent(int32_t x, int32_t y);
  MouseEvent(int32_t scrollAmount);
  MouseEvent(int32_t button, bool press);

  Type mType;

  union {
    // x-position for eMove, x-direction for eScroll
    int32_t mX;

    // only used for ePress and eRelease
    Button mButton;
  };

  // y-position for eMove, y-direction for eScroll
  int32_t mY;
};

std::ostream& operator<<(std::ostream& os, MouseEvent const& e);

} // namespace Illusion::Input

#endif // ILLUSION_INPUT_MOUSE_EVENT_HPP
