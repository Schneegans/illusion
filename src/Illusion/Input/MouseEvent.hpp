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

// ---------------------------------------------------------------------------------------- includes
#include "Enums.hpp"

namespace Illusion::Input {

struct MouseEvent {

  enum class Type { eMove, eScroll, ePress, eRelease, eLeave };

  // -------------------------------------------------------------------------------- public methods
  MouseEvent();
  MouseEvent(int x, int y);
  MouseEvent(int scrollAmount);
  MouseEvent(int button, bool press);

  // -------------------------------------------------------------------------------- public members
  // either MOVE, SCROLL, PRESS or RELEASE
  Type mType;

  union {
    // x-position for MOVE, x-direction for SCROLL
    int mX;

    // only used for PRESS and RELEASE
    Button mButton;
  };

  // y-position for MOVE, y-direction for SCROLL
  int mY;
};

std::ostream& operator<<(std::ostream& os, MouseEvent const& e);

} // namespace Illusion::Input

#endif // ILLUSION_INPUT_MOUSE_EVENT_HPP
