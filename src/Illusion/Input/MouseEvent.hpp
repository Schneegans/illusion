////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_INPUT_MOUSE_EVENT_HPP
#define ILLUSION_INPUT_MOUSE_EVENT_HPP

#include "Enums.hpp"

namespace Illusion::Input {

struct MouseEvent {

  enum class Type { eMove, eScroll, ePress, eRelease, eLeave };

  MouseEvent();
  MouseEvent(int x, int y);
  MouseEvent(int scrollAmount);
  MouseEvent(int button, bool press);

  Type mType;

  union {
    // x-position for eMove, x-direction for eScroll
    int mX;

    // only used for ePress and eRelease
    Button mButton;
  };

  // y-position for eMove, y-direction for eScroll
  int mY;
};

std::ostream& operator<<(std::ostream& os, MouseEvent const& e);

} // namespace Illusion::Input

#endif // ILLUSION_INPUT_MOUSE_EVENT_HPP
