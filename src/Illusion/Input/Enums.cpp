////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Enums.hpp"

#include "../Core/EnumCast.hpp"

#include <iostream>

namespace Illusion::Input {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, Key key) {
  switch (key) {
  case Key::eBackspace:
    os << "Backspace";
    break;
  case Key::eTab:
    os << "Tab";
    break;
  case Key::eClear:
    os << "Clear";
    break;
  case Key::eReturn:
    os << "Return";
    break;
  case Key::eShift:
    os << "Shift";
    break;
  case Key::eControl:
    os << "Control";
    break;
  case Key::eAlt:
    os << "Alt";
    break;
  case Key::ePause:
    os << "Pause";
    break;
  case Key::eCapsLock:
    os << "CapsLock";
    break;
  case Key::eKana:
    os << "Kana";
    break;
  case Key::eJunja:
    os << "Junja";
    break;
  case Key::eFinal:
    os << "Final";
    break;
  case Key::eHanja:
    os << "Hanja";
    break;
  case Key::eEscape:
    os << "Escape";
    break;
  case Key::eConvert:
    os << "Convert";
    break;
  case Key::eNonconvert:
    os << "Nonconvert";
    break;
  case Key::eAccept:
    os << "Accept";
    break;
  case Key::eModechange:
    os << "Modechange";
    break;
  case Key::eSpace:
    os << "Space";
    break;
  case Key::ePageUp:
    os << "PageUp";
    break;
  case Key::ePageDown:
    os << "PageDown";
    break;
  case Key::eEnd:
    os << "End";
    break;
  case Key::eHome:
    os << "Home";
    break;
  case Key::eLeft:
    os << "Left";
    break;
  case Key::eUp:
    os << "Up";
    break;
  case Key::eRight:
    os << "Right";
    break;
  case Key::eDown:
    os << "Down";
    break;
  case Key::eSelect:
    os << "Select";
    break;
  case Key::ePrint:
    os << "Print";
    break;
  case Key::eExecute:
    os << "Execute";
    break;
  case Key::ePrintScreen:
    os << "PrintScreen";
    break;
  case Key::eInsert:
    os << "Insert";
    break;
  case Key::eDelete:
    os << "Delete";
    break;
  case Key::eHelp:
    os << "Help";
    break;
  case Key::e0:
    os << "0";
    break;
  case Key::e1:
    os << "1";
    break;
  case Key::e2:
    os << "2";
    break;
  case Key::e3:
    os << "3";
    break;
  case Key::e4:
    os << "4";
    break;
  case Key::e5:
    os << "5";
    break;
  case Key::e6:
    os << "6";
    break;
  case Key::e7:
    os << "7";
    break;
  case Key::e8:
    os << "8";
    break;
  case Key::e9:
    os << "9";
    break;
  case Key::eA:
    os << "A";
    break;
  case Key::eB:
    os << "B";
    break;
  case Key::eC:
    os << "C";
    break;
  case Key::eD:
    os << "D";
    break;
  case Key::eE:
    os << "E";
    break;
  case Key::eF:
    os << "F";
    break;
  case Key::eG:
    os << "G";
    break;
  case Key::eH:
    os << "H";
    break;
  case Key::eI:
    os << "I";
    break;
  case Key::eJ:
    os << "J";
    break;
  case Key::eK:
    os << "K";
    break;
  case Key::eL:
    os << "L";
    break;
  case Key::eM:
    os << "M";
    break;
  case Key::eN:
    os << "N";
    break;
  case Key::eO:
    os << "O";
    break;
  case Key::eP:
    os << "P";
    break;
  case Key::eQ:
    os << "Q";
    break;
  case Key::eR:
    os << "R";
    break;
  case Key::eS:
    os << "S";
    break;
  case Key::eT:
    os << "T";
    break;
  case Key::eU:
    os << "U";
    break;
  case Key::eV:
    os << "V";
    break;
  case Key::eW:
    os << "W";
    break;
  case Key::eX:
    os << "X";
    break;
  case Key::eY:
    os << "Y";
    break;
  case Key::eZ:
    os << "Z";
    break;
  case Key::eLeftSuper:
    os << "LeftSuper";
    break;
  case Key::eRightSuper:
    os << "RightSuper";
    break;
  case Key::eApps:
    os << "Apps";
    break;
  case Key::eSleep:
    os << "Sleep";
    break;
  case Key::eKp0:
    os << "Kp0";
    break;
  case Key::eKp1:
    os << "Kp1";
    break;
  case Key::eKp2:
    os << "Kp2";
    break;
  case Key::eKp3:
    os << "Kp3";
    break;
  case Key::eKp4:
    os << "Kp4";
    break;
  case Key::eKp5:
    os << "Kp5";
    break;
  case Key::eKp6:
    os << "Kp6";
    break;
  case Key::eKp7:
    os << "Kp7";
    break;
  case Key::eKp8:
    os << "Kp8";
    break;
  case Key::eKp9:
    os << "Kp9";
    break;
  case Key::eKpMultiply:
    os << "KpMultiply";
    break;
  case Key::eKpAdd:
    os << "KpAdd";
    break;
  case Key::eKpSeparator:
    os << "KpSeparator";
    break;
  case Key::eKpSubtract:
    os << "KpSubtract";
    break;
  case Key::eKpDecimal:
    os << "KpDecimal";
    break;
  case Key::eKpDivide:
    os << "KpDivide";
    break;
  case Key::eF1:
    os << "F1";
    break;
  case Key::eF2:
    os << "F2";
    break;
  case Key::eF3:
    os << "F3";
    break;
  case Key::eF4:
    os << "F4";
    break;
  case Key::eF5:
    os << "F5";
    break;
  case Key::eF6:
    os << "F6";
    break;
  case Key::eF7:
    os << "F7";
    break;
  case Key::eF8:
    os << "F8";
    break;
  case Key::eF9:
    os << "F9";
    break;
  case Key::eF10:
    os << "F10";
    break;
  case Key::eF11:
    os << "F11";
    break;
  case Key::eF12:
    os << "F12";
    break;
  case Key::eF13:
    os << "F13";
    break;
  case Key::eF14:
    os << "F14";
    break;
  case Key::eF15:
    os << "F15";
    break;
  case Key::eF16:
    os << "F16";
    break;
  case Key::eF17:
    os << "F17";
    break;
  case Key::eF18:
    os << "F18";
    break;
  case Key::eF19:
    os << "F19";
    break;
  case Key::eF20:
    os << "F20";
    break;
  case Key::eF21:
    os << "F21";
    break;
  case Key::eF22:
    os << "F22";
    break;
  case Key::eF23:
    os << "F23";
    break;
  case Key::eF24:
    os << "F24";
    break;
  case Key::eNumLock:
    os << "NumLock";
    break;
  case Key::eScrollLock:
    os << "ScrollLock";
    break;
  case Key::eLeftShift:
    os << "LeftShift";
    break;
  case Key::eRightShift:
    os << "RightShift";
    break;
  case Key::eLeftControl:
    os << "LeftControl";
    break;
  case Key::eRightControl:
    os << "RightControl";
    break;
  case Key::eLeftMenu:
    os << "LeftMenu";
    break;
  case Key::eRightMenu:
    os << "RightMenu";
    break;
  case Key::eBrowserBack:
    os << "BrowserBack";
    break;
  case Key::eBrowserForward:
    os << "BrowserForward";
    break;
  case Key::eBrowserRefresh:
    os << "BrowserRefresh";
    break;
  case Key::eBrowserStop:
    os << "BrowserStop";
    break;
  case Key::eBrowserSearch:
    os << "BrowserSearch";
    break;
  case Key::eBrowserFavorites:
    os << "BrowserFavorites";
    break;
  case Key::eBrowserHome:
    os << "BrowserHome";
    break;
  case Key::eVolumeMute:
    os << "VolumeMute";
    break;
  case Key::eVolumeDown:
    os << "VolumeDown";
    break;
  case Key::eVolumeUp:
    os << "VolumeUp";
    break;
  case Key::eMediaNextTrack:
    os << "MediaNextTrack";
    break;
  case Key::eMediaPrevTrack:
    os << "MediaPrevTrack";
    break;
  case Key::eMediaStop:
    os << "MediaStop";
    break;
  case Key::eMediaPlayPause:
    os << "MediaPlayPause";
    break;
  case Key::eMediaLaunchMail:
    os << "MediaLaunchMail";
    break;
  case Key::eMediaLaunchMediaSelect:
    os << "MediaLaunchMediaSelect";
    break;
  case Key::eMediaLaunchApp1:
    os << "MediaLaunchApp1";
    break;
  case Key::eMediaLaunchApp2:
    os << "MediaLaunchApp2";
    break;
  case Key::ePlus:
    os << "Plus";
    break;
  case Key::eComma:
    os << "Comma";
    break;
  case Key::eMinus:
    os << "Minus";
    break;
  case Key::ePeriod:
    os << "Period";
    break;
  case Key::eOem1:
    os << "Oem1";
    break;
  case Key::eOem2:
    os << "Oem2";
    break;
  case Key::eOem3:
    os << "Oem3";
    break;
  case Key::eOem4:
    os << "Oem4";
    break;
  case Key::eOem5:
    os << "Oem5";
    break;
  case Key::eOem6:
    os << "Oem6";
    break;
  case Key::eOem7:
    os << "Oem7";
    break;
  case Key::eOem8:
    os << "Oem8";
    break;
  case Key::eOem102:
    os << "Oem102";
    break;
  case Key::eUnknown:
    os << "Unknown";
    break;
  default:
    os << "ERROR";
    break;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, Button button) {
  os << "BUTTON_" << Core::enumCast(button);
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, JoystickId id) {
  os << "JOYSTICK_" << Core::enumCast(id);
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, JoystickAxisId axis) {
  os << "JOYSTICK_AXIS_" << Core::enumCast(axis);
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, JoystickButtonId button) {
  os << "JOYSTICK_BUTTON_" << Core::enumCast(button);
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, Xbox360ControllerButton button) {
  switch (button) {
  case Xbox360ControllerButton::eXboxA:
    os << "XboxA";
    return os;
  case Xbox360ControllerButton::eXboxB:
    os << "XboxB";
    return os;
  case Xbox360ControllerButton::eXboxX:
    os << "XboxX";
    return os;
  case Xbox360ControllerButton::eXboxY:
    os << "XboxY";
    return os;
  case Xbox360ControllerButton::eXboxBumperLeft:
    os << "XboxBumperLeft";
    return os;
  case Xbox360ControllerButton::eXboxBumperRight:
    os << "XboxBumperRight";
    return os;
  case Xbox360ControllerButton::eXboxBack:
    os << "XboxBack";
    return os;
  case Xbox360ControllerButton::eXboxStart:
    os << "XboxStart";
    return os;
  case Xbox360ControllerButton::eXboxXbox:
    os << "XboxXbox";
    return os;
  case Xbox360ControllerButton::eXboxLeftStickButton:
    os << "XboxLeftStickButton";
    return os;
  case Xbox360ControllerButton::eXboxRightStickButton:
    os << "XboxRightStickButton";
    return os;
  case Xbox360ControllerButton::eXboxPadUp:
    os << "XboxPadUp";
    return os;
  case Xbox360ControllerButton::eXboxPadRight:
    os << "XboxPadRight";
    return os;
  case Xbox360ControllerButton::eXboxPadDown:
    os << "XboxPadDown";
    return os;
  case Xbox360ControllerButton::eXboxPadLeft:
    os << "XboxPadLeft";
    return os;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, Xbox360ControllerAxis axis) {
  switch (axis) {
  case Xbox360ControllerAxis::eXboxLeftStickX:
    os << "XboxLeftStickX";
    return os;
  case Xbox360ControllerAxis::eXboxLeftStickY:
    os << "XboxLeftStickY";
    return os;
  case Xbox360ControllerAxis::eXboxLeftTrigger:
    os << "XboxLeftTrigger";
    return os;
  case Xbox360ControllerAxis::eXboxRightStickX:
    os << "XboxRightStickX";
    return os;
  case Xbox360ControllerAxis::eXboxRightStickY:
    os << "XboxRightStickY";
    return os;
  case Xbox360ControllerAxis::eXboxRightTrigger:
    os << "XboxRightTrigger";
    return os;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Input
