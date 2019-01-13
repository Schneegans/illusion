////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MouseEvent.hpp"

#include <GLFW/glfw3.h>

namespace Illusion::Input {

////////////////////////////////////////////////////////////////////////////////////////////////////

MouseEvent::MouseEvent()
    : mType(Type::eMove)
    , mX(0)
    , mY(0) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MouseEvent::MouseEvent(int x, int y) {
  mType = MouseEvent::Type::eMove;
  mX    = x;
  mY    = y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MouseEvent::MouseEvent(int scrollAmount) {
  mType = MouseEvent::Type::eScroll;
  mY    = scrollAmount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MouseEvent::MouseEvent(int button, bool press) {
  if (button == GLFW_MOUSE_BUTTON_LEFT)
    mButton = Button::eButton1;
  else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    mButton = Button::eButton2;
  else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    mButton = Button::eButton3;
  else
    mButton = (Button)button;

  if (press)
    mType = MouseEvent::Type::ePress;
  else
    mType = MouseEvent::Type::eRelease;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, MouseEvent const& e) {
  switch (e.mType) {
  case MouseEvent::Type::eMove:
    os << "move " << e.mX << " " << e.mY;
    return os;
  case MouseEvent::Type::eScroll:
    os << "scroll " << e.mX << " " << e.mY;
    return os;
  case MouseEvent::Type::ePress:
    os << "press " << e.mButton;
    return os;
  case MouseEvent::Type::eRelease:
    os << "release " << e.mButton;
    return os;
  case MouseEvent::Type::eLeave:
    os << "leave";
    return os;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Input
