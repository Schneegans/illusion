////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "MouseEvent.hpp"

#include <GLFW/glfw3.h>

namespace Illusion::Input {

MouseEvent::MouseEvent()
  : mType(Type::MOVE)
  , mX(0)
  , mY(0) {}

MouseEvent::MouseEvent(int x, int y) {
  mType = MouseEvent::Type::MOVE;
  mX    = x;
  mY    = y;
}

MouseEvent::MouseEvent(int scrollAmount) {
  mType = MouseEvent::Type::SCROLL;
  mY    = scrollAmount;
}

MouseEvent::MouseEvent(int button, bool press) {
  if (button == GLFW_MOUSE_BUTTON_LEFT)
    mButton = Button::BUTTON_1;
  else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    mButton = Button::BUTTON_2;
  else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    mButton = Button::BUTTON_3;
  else
    mButton = (Button)button;

  if (press)
    mType = MouseEvent::Type::PRESS;
  else
    mType = MouseEvent::Type::RELEASE;
}

std::ostream& operator<<(std::ostream& os, MouseEvent const& e) {
  switch (e.mType) {
  case MouseEvent::Type::MOVE:
    os << "MOVE " << e.mX << " " << e.mY;
    return os;
  case MouseEvent::Type::SCROLL:
    os << "SCROLL " << e.mX << " " << e.mY;
    return os;
  case MouseEvent::Type::PRESS:
    os << "PRESS " << e.mButton;
    return os;
  case MouseEvent::Type::RELEASE:
    os << "RELEASE " << e.mButton;
    return os;
  case MouseEvent::Type::LEAVE:
    os << "LEAVE";
    return os;
  }
  return os;
}

} // namespace Illusion::Input
