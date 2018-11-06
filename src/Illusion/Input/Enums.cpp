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
#include "Enums.hpp"

#include <iostream>

namespace Illusion::Input {

std::ostream& operator<<(std::ostream& os, Key key) {
  switch (key) {
  case Key::BACKSPACE:
    os << "BACKSPACE";
    break;
  case Key::TAB:
    os << "TAB";
    break;
  case Key::CLEAR:
    os << "CLEAR";
    break;
  case Key::RETURN:
    os << "RETURN";
    break;
  case Key::SHIFT:
    os << "SHIFT";
    break;
  case Key::CONTROL:
    os << "CONTROL";
    break;
  case Key::ALT:
    os << "ALT";
    break;
  case Key::PAUSE:
    os << "PAUSE";
    break;
  case Key::CAPS_LOCK:
    os << "CAPS_LOCK";
    break;
  case Key::KANA:
    os << "KANA";
    break;
  case Key::JUNJA:
    os << "JUNJA";
    break;
  case Key::FINAL:
    os << "FINAL";
    break;
  case Key::HANJA:
    os << "HANJA";
    break;
  case Key::ESCAPE:
    os << "ESCAPE";
    break;
  case Key::CONVERT:
    os << "CONVERT";
    break;
  case Key::NONCONVERT:
    os << "NONCONVERT";
    break;
  case Key::ACCEPT:
    os << "ACCEPT";
    break;
  case Key::MODECHANGE:
    os << "MODECHANGE";
    break;
  case Key::SPACE:
    os << "SPACE";
    break;
  case Key::PAGE_UP:
    os << "PAGE_UP";
    break;
  case Key::PAGE_DOWN:
    os << "PAGE_DOWN";
    break;
  case Key::END:
    os << "END";
    break;
  case Key::HOME:
    os << "HOME";
    break;
  case Key::LEFT:
    os << "LEFT";
    break;
  case Key::UP:
    os << "UP";
    break;
  case Key::RIGHT:
    os << "RIGHT";
    break;
  case Key::DOWN:
    os << "DOWN";
    break;
  case Key::SELECT:
    os << "SELECT";
    break;
  case Key::PRINT:
    os << "PRINT";
    break;
  case Key::EXECUTE:
    os << "EXECUTE";
    break;
  case Key::PRINT_SCREEN:
    os << "PRINT_SCREEN";
    break;
  case Key::INSERT:
    os << "INSERT";
    break;
  case Key::KEY_DELETE:
    os << "DELETE";
    break;
  case Key::HELP:
    os << "HELP";
    break;
  case Key::KEY_0:
    os << "KEY_0";
    break;
  case Key::KEY_1:
    os << "KEY_1";
    break;
  case Key::KEY_2:
    os << "KEY_2";
    break;
  case Key::KEY_3:
    os << "KEY_3";
    break;
  case Key::KEY_4:
    os << "KEY_4";
    break;
  case Key::KEY_5:
    os << "KEY_5";
    break;
  case Key::KEY_6:
    os << "KEY_6";
    break;
  case Key::KEY_7:
    os << "KEY_7";
    break;
  case Key::KEY_8:
    os << "KEY_8";
    break;
  case Key::KEY_9:
    os << "KEY_9";
    break;
  case Key::KEY_A:
    os << "KEY_A";
    break;
  case Key::KEY_B:
    os << "KEY_B";
    break;
  case Key::KEY_C:
    os << "KEY_C";
    break;
  case Key::KEY_D:
    os << "KEY_D";
    break;
  case Key::KEY_E:
    os << "KEY_E";
    break;
  case Key::KEY_F:
    os << "KEY_F";
    break;
  case Key::KEY_G:
    os << "KEY_G";
    break;
  case Key::KEY_H:
    os << "KEY_H";
    break;
  case Key::KEY_I:
    os << "KEY_I";
    break;
  case Key::KEY_J:
    os << "KEY_J";
    break;
  case Key::KEY_K:
    os << "KEY_K";
    break;
  case Key::KEY_L:
    os << "KEY_L";
    break;
  case Key::KEY_M:
    os << "KEY_M";
    break;
  case Key::KEY_N:
    os << "KEY_N";
    break;
  case Key::KEY_O:
    os << "KEY_O";
    break;
  case Key::KEY_P:
    os << "KEY_P";
    break;
  case Key::KEY_Q:
    os << "KEY_Q";
    break;
  case Key::KEY_R:
    os << "KEY_R";
    break;
  case Key::KEY_S:
    os << "KEY_S";
    break;
  case Key::KEY_T:
    os << "KEY_T";
    break;
  case Key::KEY_U:
    os << "KEY_U";
    break;
  case Key::KEY_V:
    os << "KEY_V";
    break;
  case Key::KEY_W:
    os << "KEY_W";
    break;
  case Key::KEY_X:
    os << "KEY_X";
    break;
  case Key::KEY_Y:
    os << "KEY_Y";
    break;
  case Key::KEY_Z:
    os << "KEY_Z";
    break;
  case Key::LEFT_SUPER:
    os << "LEFT_SUPER";
    break;
  case Key::RIGHT_SUPER:
    os << "RIGHT_SUPER";
    break;
  case Key::APPS:
    os << "APPS";
    break;
  case Key::SLEEP:
    os << "SLEEP";
    break;
  case Key::KP_0:
    os << "KP_0";
    break;
  case Key::KP_1:
    os << "KP_1";
    break;
  case Key::KP_2:
    os << "KP_2";
    break;
  case Key::KP_3:
    os << "KP_3";
    break;
  case Key::KP_4:
    os << "KP_4";
    break;
  case Key::KP_5:
    os << "KP_5";
    break;
  case Key::KP_6:
    os << "KP_6";
    break;
  case Key::KP_7:
    os << "KP_7";
    break;
  case Key::KP_8:
    os << "KP_8";
    break;
  case Key::KP_9:
    os << "KP_9";
    break;
  case Key::KP_MULTIPLY:
    os << "KP_MULTIPLY";
    break;
  case Key::KP_ADD:
    os << "KP_ADD";
    break;
  case Key::KP_SEPARATOR:
    os << "KP_SEPARATOR";
    break;
  case Key::KP_SUBTRACT:
    os << "KP_SUBTRACT";
    break;
  case Key::KP_DECIMAL:
    os << "KP_DECIMAL";
    break;
  case Key::KP_DIVIDE:
    os << "KP_DIVIDE";
    break;
  case Key::F1:
    os << "F1";
    break;
  case Key::F2:
    os << "F2";
    break;
  case Key::F3:
    os << "F3";
    break;
  case Key::F4:
    os << "F4";
    break;
  case Key::F5:
    os << "F5";
    break;
  case Key::F6:
    os << "F6";
    break;
  case Key::F7:
    os << "F7";
    break;
  case Key::F8:
    os << "F8";
    break;
  case Key::F9:
    os << "F9";
    break;
  case Key::F10:
    os << "F10";
    break;
  case Key::F11:
    os << "F11";
    break;
  case Key::F12:
    os << "F12";
    break;
  case Key::F13:
    os << "F13";
    break;
  case Key::F14:
    os << "F14";
    break;
  case Key::F15:
    os << "F15";
    break;
  case Key::F16:
    os << "F16";
    break;
  case Key::F17:
    os << "F17";
    break;
  case Key::F18:
    os << "F18";
    break;
  case Key::F19:
    os << "F19";
    break;
  case Key::F20:
    os << "F20";
    break;
  case Key::F21:
    os << "F21";
    break;
  case Key::F22:
    os << "F22";
    break;
  case Key::F23:
    os << "F23";
    break;
  case Key::F24:
    os << "F24";
    break;
  case Key::NUM_LOCK:
    os << "NUM_LOCK";
    break;
  case Key::SCROLL_LOCK:
    os << "SCROLL_LOCK";
    break;
  case Key::LEFT_SHIFT:
    os << "LEFT_SHIFT";
    break;
  case Key::RIGHT_SHIFT:
    os << "RIGHT_SHIFT";
    break;
  case Key::LEFT_CONTROL:
    os << "LEFT_CONTROL";
    break;
  case Key::RIGHT_CONTROL:
    os << "RIGHT_CONTROL";
    break;
  case Key::LEFT_MENU:
    os << "LEFT_MENU";
    break;
  case Key::RIGHT_MENU:
    os << "RIGHT_MENU";
    break;
  case Key::BROWSER_BACK:
    os << "BROWSER_BACK";
    break;
  case Key::BROWSER_FORWARD:
    os << "BROWSER_FORWARD";
    break;
  case Key::BROWSER_REFRESH:
    os << "BROWSER_REFRESH";
    break;
  case Key::BROWSER_STOP:
    os << "BROWSER_STOP";
    break;
  case Key::BROWSER_SEARCH:
    os << "BROWSER_SEARCH";
    break;
  case Key::BROWSER_FAVORITES:
    os << "BROWSER_FAVORITES";
    break;
  case Key::BROWSER_HOME:
    os << "BROWSER_HOME";
    break;
  case Key::VOLUME_MUTE:
    os << "VOLUME_MUTE";
    break;
  case Key::VOLUME_DOWN:
    os << "VOLUME_DOWN";
    break;
  case Key::VOLUME_UP:
    os << "VOLUME_UP";
    break;
  case Key::MEDIA_NEXT_TRACK:
    os << "MEDIA_NEXT_TRACK";
    break;
  case Key::MEDIA_PREV_TRACK:
    os << "MEDIA_PREV_TRACK";
    break;
  case Key::MEDIA_STOP:
    os << "MEDIA_STOP";
    break;
  case Key::MEDIA_PLAY_PAUSE:
    os << "MEDIA_PLAY_PAUSE";
    break;
  case Key::MEDIA_LAUNCH_MAIL:
    os << "MEDIA_LAUNCH_MAIL";
    break;
  case Key::MEDIA_LAUNCH_MEDIA_SELECT:
    os << "MEDIA_LAUNCH_MEDIA_SELECT";
    break;
  case Key::MEDIA_LAUNCH_APP1:
    os << "MEDIA_LAUNCH_APP1";
    break;
  case Key::MEDIA_LAUNCH_APP2:
    os << "MEDIA_LAUNCH_APP2";
    break;
  case Key::PLUS:
    os << "PLUS";
    break;
  case Key::COMMA:
    os << "COMMA";
    break;
  case Key::MINUS:
    os << "MINUS";
    break;
  case Key::PERIOD:
    os << "PERIOD";
    break;
  case Key::OEM_1:
    os << "OEM_1";
    break;
  case Key::OEM_2:
    os << "OEM_2";
    break;
  case Key::OEM_3:
    os << "OEM_3";
    break;
  case Key::OEM_4:
    os << "OEM_4";
    break;
  case Key::OEM_5:
    os << "OEM_5";
    break;
  case Key::OEM_6:
    os << "OEM_6";
    break;
  case Key::OEM_7:
    os << "OEM_7";
    break;
  case Key::OEM_8:
    os << "OEM_8";
    break;
  case Key::OEM_102:
    os << "OEM_102";
    break;
  case Key::UNKNOWN:
    os << "UNKNOWN";
    break;
  default:
    os << "ERROR";
    break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, Button button) {
  os << "BUTTON_" << (int)button;
  return os;
}

std::ostream& operator<<(std::ostream& os, JoystickId id) {
  os << "JOYSTICK_" << (int)id;
  return os;
}

std::ostream& operator<<(std::ostream& os, JoystickAxisId axis) {
  os << "JOYSTICK_AXIS_" << (int)axis;
  return os;
}

std::ostream& operator<<(std::ostream& os, JoystickButtonId button) {
  os << "JOYSTICK_BUTTON_" << (int)button;
  return os;
}

std::ostream& operator<<(std::ostream& os, Xbox360ControllerButton button) {
  switch (button) {
  case Xbox360ControllerButton::XBOX_A:
    os << "XBOX_A";
    return os;
  case Xbox360ControllerButton::XBOX_B:
    os << "XBOX_B";
    return os;
  case Xbox360ControllerButton::XBOX_X:
    os << "XBOX_X";
    return os;
  case Xbox360ControllerButton::XBOX_Y:
    os << "XBOX_Y";
    return os;
  case Xbox360ControllerButton::XBOX_BUMPER_LEFT:
    os << "XBOX_BUMPER_LEFT";
    return os;
  case Xbox360ControllerButton::XBOX_BUMPER_RIGHT:
    os << "XBOX_BUMPER_RIGHT";
    return os;
  case Xbox360ControllerButton::XBOX_BACK:
    os << "XBOX_BACK";
    return os;
  case Xbox360ControllerButton::XBOX_START:
    os << "XBOX_START";
    return os;
  case Xbox360ControllerButton::XBOX_XBOX:
    os << "XBOX_XBOX";
    return os;
  case Xbox360ControllerButton::XBOX_LEFT_STICK_BUTTON:
    os << "XBOX_LEFT_STICK_BUTTON";
    return os;
  case Xbox360ControllerButton::XBOX_RIGHT_STICK_BUTTON:
    os << "XBOX_RIGHT_STICK_BUTTON";
    return os;
  case Xbox360ControllerButton::XBOX_PAD_UP:
    os << "XBOX_PAD_UP";
    return os;
  case Xbox360ControllerButton::XBOX_PAD_RIGHT:
    os << "XBOX_PAD_RIGHT";
    return os;
  case Xbox360ControllerButton::XBOX_PAD_DOWN:
    os << "XBOX_PAD_DOWN";
    return os;
  case Xbox360ControllerButton::XBOX_PAD_LEFT:
    os << "XBOX_PAD_LEFT";
    return os;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, Xbox360ControllerAxis axis) {
  switch (axis) {
  case Xbox360ControllerAxis::XBOX_LEFT_STICK_X:
    os << "XBOX_LEFT_STICK_X";
    return os;
  case Xbox360ControllerAxis::XBOX_LEFT_STICK_Y:
    os << "XBOX_LEFT_STICK_Y";
    return os;
  case Xbox360ControllerAxis::XBOX_LEFT_TRIGGER:
    os << "XBOX_LEFT_TRIGGER";
    return os;
  case Xbox360ControllerAxis::XBOX_RIGHT_STICK_X:
    os << "XBOX_RIGHT_STICK_X";
    return os;
  case Xbox360ControllerAxis::XBOX_RIGHT_STICK_Y:
    os << "XBOX_RIGHT_STICK_Y";
    return os;
  case Xbox360ControllerAxis::XBOX_RIGHT_TRIGGER:
    os << "XBOX_RIGHT_TRIGGER";
    return os;
  }
  return os;
}

} // namespace Illusion::Input
