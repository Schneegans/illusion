////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_INPUT_ENUMS_HPP
#define ILLUSION_INPUT_ENUMS_HPP

// ---------------------------------------------------------------------------------------- includes
#include <iostream>

namespace Illusion::Input {

enum class JoystickId {
  JOYSTICK_0   = 0,
  JOYSTICK_1   = 1,
  JOYSTICK_2   = 2,
  JOYSTICK_3   = 3,
  JOYSTICK_4   = 4,
  JOYSTICK_5   = 5,
  JOYSTICK_6   = 6,
  JOYSTICK_7   = 7,
  JOYSTICK_8   = 8,
  JOYSTICK_9   = 9,
  JOYSTICK_10  = 10,
  JOYSTICK_11  = 11,
  JOYSTICK_12  = 12,
  JOYSTICK_13  = 13,
  JOYSTICK_14  = 14,
  JOYSTICK_15  = 15,
  JOYSTICK_NUM = 16
};

enum class JoystickAxisId {
  JOYSTICK_AXIS_0   = 0,
  JOYSTICK_AXIS_1   = 1,
  JOYSTICK_AXIS_2   = 2,
  JOYSTICK_AXIS_3   = 3,
  JOYSTICK_AXIS_4   = 4,
  JOYSTICK_AXIS_5   = 5,
  JOYSTICK_AXIS_6   = 6,
  JOYSTICK_AXIS_7   = 7,
  JOYSTICK_AXIS_8   = 8,
  JOYSTICK_AXIS_9   = 9,
  JOYSTICK_AXIS_10  = 10,
  JOYSTICK_AXIS_11  = 11,
  JOYSTICK_AXIS_12  = 12,
  JOYSTICK_AXIS_13  = 13,
  JOYSTICK_AXIS_14  = 14,
  JOYSTICK_AXIS_15  = 15,
  JOYSTICK_AXIS_16  = 16,
  JOYSTICK_AXIS_17  = 17,
  JOYSTICK_AXIS_18  = 18,
  JOYSTICK_AXIS_19  = 19,
  JOYSTICK_AXIS_20  = 20,
  JOYSTICK_AXIS_21  = 21,
  JOYSTICK_AXIS_22  = 22,
  JOYSTICK_AXIS_23  = 23,
  JOYSTICK_AXIS_24  = 24,
  JOYSTICK_AXIS_25  = 25,
  JOYSTICK_AXIS_26  = 26,
  JOYSTICK_AXIS_27  = 27,
  JOYSTICK_AXIS_28  = 28,
  JOYSTICK_AXIS_29  = 29,
  JOYSTICK_AXIS_30  = 30,
  JOYSTICK_AXIS_31  = 31,
  JOYSTICK_AXIS_NUM = 32
};

enum class JoystickButtonId {
  JOYSTICK_BUTTON_0   = 0,
  JOYSTICK_BUTTON_1   = 1,
  JOYSTICK_BUTTON_2   = 2,
  JOYSTICK_BUTTON_3   = 3,
  JOYSTICK_BUTTON_4   = 4,
  JOYSTICK_BUTTON_5   = 5,
  JOYSTICK_BUTTON_6   = 6,
  JOYSTICK_BUTTON_7   = 7,
  JOYSTICK_BUTTON_8   = 8,
  JOYSTICK_BUTTON_9   = 9,
  JOYSTICK_BUTTON_10  = 10,
  JOYSTICK_BUTTON_11  = 11,
  JOYSTICK_BUTTON_12  = 12,
  JOYSTICK_BUTTON_13  = 13,
  JOYSTICK_BUTTON_14  = 14,
  JOYSTICK_BUTTON_15  = 15,
  JOYSTICK_BUTTON_16  = 16,
  JOYSTICK_BUTTON_17  = 17,
  JOYSTICK_BUTTON_18  = 18,
  JOYSTICK_BUTTON_19  = 19,
  JOYSTICK_BUTTON_20  = 20,
  JOYSTICK_BUTTON_21  = 21,
  JOYSTICK_BUTTON_22  = 22,
  JOYSTICK_BUTTON_23  = 23,
  JOYSTICK_BUTTON_24  = 24,
  JOYSTICK_BUTTON_25  = 25,
  JOYSTICK_BUTTON_26  = 26,
  JOYSTICK_BUTTON_27  = 27,
  JOYSTICK_BUTTON_28  = 28,
  JOYSTICK_BUTTON_29  = 29,
  JOYSTICK_BUTTON_30  = 30,
  JOYSTICK_BUTTON_31  = 31,
  JOYSTICK_BUTTON_NUM = 32
};

enum class Xbox360ControllerButton {
  XBOX_A                  = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_0),
  XBOX_B                  = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_1),
  XBOX_X                  = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_2),
  XBOX_Y                  = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_3),
  XBOX_BUMPER_LEFT        = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_4),
  XBOX_BUMPER_RIGHT       = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_5),
  XBOX_BACK               = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_6),
  XBOX_START              = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_7),
  XBOX_XBOX               = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_8),
  XBOX_LEFT_STICK_BUTTON  = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_9),
  XBOX_RIGHT_STICK_BUTTON = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_10),
  XBOX_PAD_UP             = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_11),
  XBOX_PAD_RIGHT          = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_12),
  XBOX_PAD_DOWN           = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_13),
  XBOX_PAD_LEFT           = static_cast<int>(JoystickButtonId::JOYSTICK_BUTTON_14)
};

enum class Xbox360ControllerAxis {
  XBOX_LEFT_STICK_X  = static_cast<int>(JoystickAxisId::JOYSTICK_AXIS_0),
  XBOX_LEFT_STICK_Y  = static_cast<int>(JoystickAxisId::JOYSTICK_AXIS_1),
  XBOX_LEFT_TRIGGER  = static_cast<int>(JoystickAxisId::JOYSTICK_AXIS_2),
  XBOX_RIGHT_STICK_X = static_cast<int>(JoystickAxisId::JOYSTICK_AXIS_3),
  XBOX_RIGHT_STICK_Y = static_cast<int>(JoystickAxisId::JOYSTICK_AXIS_4),
  XBOX_RIGHT_TRIGGER = static_cast<int>(JoystickAxisId::JOYSTICK_AXIS_5)
};

enum class Button {
  BUTTON_1 = 0,
  BUTTON_2 = 1,
  BUTTON_3 = 2,
  BUTTON_4 = 3,
  BUTTON_5 = 4,
  BUTTON_6 = 5,
  BUTTON_7 = 6,
  BUTTON_8 = 7
};

enum class Modifier {
  NONE          = 0,
  CAPS_LOCK     = 1 << 0,
  SHIFT         = 1 << 1,
  CONTROL       = 1 << 2,
  ALT           = 1 << 3,
  LEFT_BUTTON   = 1 << 4,
  MIDDLE_BUTTON = 1 << 5,
  RIGHT_BUTTON  = 1 << 6,
  COMMAND       = 1 << 7,
  NUM_LOCK      = 1 << 8,
  IS_KEY_PAD    = 1 << 9,
  IS_LEFT       = 1 << 10,
  IS_RIGHT      = 1 << 11
};

// codes based on the chromium key at
// https://github.com/adobe/webkit/blob/master/Source/WebCore/platform/WindowsKeyboardCodes.h
enum class Key {
  UNKNOWN = 0,

  BACKSPACE    = 0x08,
  TAB          = 0x09,
  CLEAR        = 0x0C,
  RETURN       = 0x0D,
  SHIFT        = 0x10,
  CONTROL      = 0x11,
  ALT          = 0x12,
  PAUSE        = 0x13,
  CAPS_LOCK    = 0x14,
  KANA         = 0x15,
  JUNJA        = 0x17,
  FINAL        = 0x18,
  HANJA        = 0x19,
  ESCAPE       = 0x1B,
  CONVERT      = 0x1C,
  NONCONVERT   = 0x1D,
  ACCEPT       = 0x1E,
  MODECHANGE   = 0x1F,
  SPACE        = 0x20,
  PAGE_UP      = 0x21,
  PAGE_DOWN    = 0x22,
  END          = 0x23,
  HOME         = 0x24,
  LEFT         = 0x25,
  UP           = 0x26,
  RIGHT        = 0x27,
  DOWN         = 0x28,
  SELECT       = 0x29,
  PRINT        = 0x2A,
  EXECUTE      = 0x2B,
  PRINT_SCREEN = 0x2C,
  INSERT       = 0x2D,
  KEY_DELETE   = 0x2E,
  HELP         = 0x2F,

  KEY_0 = 0x30,
  KEY_1 = 0x31,
  KEY_2 = 0x32,
  KEY_3 = 0x33,
  KEY_4 = 0x34,
  KEY_5 = 0x35,
  KEY_6 = 0x36,
  KEY_7 = 0x37,
  KEY_8 = 0x38,
  KEY_9 = 0x39,

  KEY_A = 0x41,
  KEY_B = 0x42,
  KEY_C = 0x43,
  KEY_D = 0x44,
  KEY_E = 0x45,
  KEY_F = 0x46,
  KEY_G = 0x47,
  KEY_H = 0x48,
  KEY_I = 0x49,
  KEY_J = 0x4A,
  KEY_K = 0x4B,
  KEY_L = 0x4C,
  KEY_M = 0x4D,
  KEY_N = 0x4E,
  KEY_O = 0x4F,
  KEY_P = 0x50,
  KEY_Q = 0x51,
  KEY_R = 0x52,
  KEY_S = 0x53,
  KEY_T = 0x54,
  KEY_U = 0x55,
  KEY_V = 0x56,
  KEY_W = 0x57,
  KEY_X = 0x58,
  KEY_Y = 0x59,
  KEY_Z = 0x5A,

  LEFT_SUPER  = 0x5B,
  RIGHT_SUPER = 0x5C,
  APPS        = 0x5D,
  SLEEP       = 0x5F,

  KP_0         = 0x60,
  KP_1         = 0x61,
  KP_2         = 0x62,
  KP_3         = 0x63,
  KP_4         = 0x64,
  KP_5         = 0x65,
  KP_6         = 0x66,
  KP_7         = 0x67,
  KP_8         = 0x68,
  KP_9         = 0x69,
  KP_MULTIPLY  = 0x6A,
  KP_ADD       = 0x6B,
  KP_SEPARATOR = 0x6C,
  KP_SUBTRACT  = 0x6D,
  KP_DECIMAL   = 0x6E,
  KP_DIVIDE    = 0x6F,

  F1  = 0x70,
  F2  = 0x71,
  F3  = 0x72,
  F4  = 0x73,
  F5  = 0x74,
  F6  = 0x75,
  F7  = 0x76,
  F8  = 0x77,
  F9  = 0x78,
  F10 = 0x79,
  F11 = 0x7A,
  F12 = 0x7B,
  F13 = 0x7C,
  F14 = 0x7D,
  F15 = 0x7E,
  F16 = 0x7F,
  F17 = 0x80,
  F18 = 0x81,
  F19 = 0x82,
  F20 = 0x83,
  F21 = 0x84,
  F22 = 0x85,
  F23 = 0x86,
  F24 = 0x87,

  NUM_LOCK      = 0x90,
  SCROLL_LOCK   = 0x91,
  LEFT_SHIFT    = 0xA0,
  RIGHT_SHIFT   = 0xA1,
  LEFT_CONTROL  = 0xA2,
  RIGHT_CONTROL = 0xA3,
  LEFT_MENU     = 0xA4,
  RIGHT_MENU    = 0xA5,

  BROWSER_BACK              = 0xA6,
  BROWSER_FORWARD           = 0xA7,
  BROWSER_REFRESH           = 0xA8,
  BROWSER_STOP              = 0xA9,
  BROWSER_SEARCH            = 0xAA,
  BROWSER_FAVORITES         = 0xAB,
  BROWSER_HOME              = 0xAC,
  VOLUME_MUTE               = 0xAD,
  VOLUME_DOWN               = 0xAE,
  VOLUME_UP                 = 0xAF,
  MEDIA_NEXT_TRACK          = 0xB0,
  MEDIA_PREV_TRACK          = 0xB1,
  MEDIA_STOP                = 0xB2,
  MEDIA_PLAY_PAUSE          = 0xB3,
  MEDIA_LAUNCH_MAIL         = 0xB4,
  MEDIA_LAUNCH_MEDIA_SELECT = 0xB5,
  MEDIA_LAUNCH_APP1         = 0xB6,
  MEDIA_LAUNCH_APP2         = 0xB7,

  PLUS   = 0xBB,
  COMMA  = 0xBC,
  MINUS  = 0xBD,
  PERIOD = 0xBE,

  OEM_1   = 0xBA, // The ';:' key
  OEM_2   = 0xBF, // The '/?' key
  OEM_3   = 0xC0, // The '`~' key
  OEM_4   = 0xDB, // The '[{' key
  OEM_5   = 0xDC, // The '\|' key
  OEM_6   = 0xDD, // The ']}' key
  OEM_7   = 0xDE, // The 'single-quote/double-quote' key
  OEM_8   = 0xDF, // Used for miscellaneous characters; it can vary by keyboard.
  OEM_102 = 0xE2  // Either the angle bracket key or the backslash key on the RT 102-key keyboard
};

std::ostream& operator<<(std::ostream& os, Key key);
std::ostream& operator<<(std::ostream& os, Button button);
std::ostream& operator<<(std::ostream& os, JoystickId id);
std::ostream& operator<<(std::ostream& os, JoystickAxisId axis);
std::ostream& operator<<(std::ostream& os, JoystickButtonId button);
std::ostream& operator<<(std::ostream& os, Xbox360ControllerButton button);
std::ostream& operator<<(std::ostream& os, Xbox360ControllerAxis axis);

} // namespace Illusion::Input

#endif // ILLUSION_INPUT_ENUMS_HPP
