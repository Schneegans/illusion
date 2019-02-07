////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_INPUT_ENUMS_HPP
#define ILLUSION_INPUT_ENUMS_HPP

#include "../Core/EnumCast.hpp"

#include <iostream>

namespace Illusion::Input {

enum class JoystickId : uint16_t {
  eJoystick0   = 0,
  eJoystick1   = 1,
  eJoystick2   = 2,
  eJoystick3   = 3,
  eJoystick4   = 4,
  eJoystick5   = 5,
  eJoystick6   = 6,
  eJoystick7   = 7,
  eJoystick8   = 8,
  eJoystick9   = 9,
  eJoystick10  = 10,
  eJoystick11  = 11,
  eJoystick12  = 12,
  eJoystick13  = 13,
  eJoystick14  = 14,
  eJoystick15  = 15,
  eJoystickNum = 16
};

enum class JoystickAxisId : uint16_t {
  eJoystickAxis0   = 0,
  eJoystickAxis1   = 1,
  eJoystickAxis2   = 2,
  eJoystickAxis3   = 3,
  eJoystickAxis4   = 4,
  eJoystickAxis5   = 5,
  eJoystickAxis6   = 6,
  eJoystickAxis7   = 7,
  eJoystickAxis8   = 8,
  eJoystickAxis9   = 9,
  eJoystickAxis10  = 10,
  eJoystickAxis11  = 11,
  eJoystickAxis12  = 12,
  eJoystickAxis13  = 13,
  eJoystickAxis14  = 14,
  eJoystickAxis15  = 15,
  eJoystickAxis16  = 16,
  eJoystickAxis17  = 17,
  eJoystickAxis18  = 18,
  eJoystickAxis19  = 19,
  eJoystickAxis20  = 20,
  eJoystickAxis21  = 21,
  eJoystickAxis22  = 22,
  eJoystickAxis23  = 23,
  eJoystickAxis24  = 24,
  eJoystickAxis25  = 25,
  eJoystickAxis26  = 26,
  eJoystickAxis27  = 27,
  eJoystickAxis28  = 28,
  eJoystickAxis29  = 29,
  eJoystickAxis30  = 30,
  eJoystickAxis31  = 31,
  eJoystickAxisNum = 32
};

enum class JoystickButtonId : uint16_t {
  eJoystickButton0   = 0,
  eJoystickButton1   = 1,
  eJoystickButton2   = 2,
  eJoystickButton3   = 3,
  eJoystickButton4   = 4,
  eJoystickButton5   = 5,
  eJoystickButton6   = 6,
  eJoystickButton7   = 7,
  eJoystickButton8   = 8,
  eJoystickButton9   = 9,
  eJoystickButton10  = 10,
  eJoystickButton11  = 11,
  eJoystickButton12  = 12,
  eJoystickButton13  = 13,
  eJoystickButton14  = 14,
  eJoystickButton15  = 15,
  eJoystickButton16  = 16,
  eJoystickButton17  = 17,
  eJoystickButton18  = 18,
  eJoystickButton19  = 19,
  eJoystickButton20  = 20,
  eJoystickButton21  = 21,
  eJoystickButton22  = 22,
  eJoystickButton23  = 23,
  eJoystickButton24  = 24,
  eJoystickButton25  = 25,
  eJoystickButton26  = 26,
  eJoystickButton27  = 27,
  eJoystickButton28  = 28,
  eJoystickButton29  = 29,
  eJoystickButton30  = 30,
  eJoystickButton31  = 31,
  eJoystickButtonNum = 32
};

enum class Xbox360ControllerButton : uint16_t {
  eXboxA                = Core::enumCast(JoystickButtonId::eJoystickButton0),
  eXboxB                = Core::enumCast(JoystickButtonId::eJoystickButton1),
  eXboxX                = Core::enumCast(JoystickButtonId::eJoystickButton2),
  eXboxY                = Core::enumCast(JoystickButtonId::eJoystickButton3),
  eXboxBumperLeft       = Core::enumCast(JoystickButtonId::eJoystickButton4),
  eXboxBumperRight      = Core::enumCast(JoystickButtonId::eJoystickButton5),
  eXboxBack             = Core::enumCast(JoystickButtonId::eJoystickButton6),
  eXboxStart            = Core::enumCast(JoystickButtonId::eJoystickButton7),
  eXboxXbox             = Core::enumCast(JoystickButtonId::eJoystickButton8),
  eXboxLeftStickButton  = Core::enumCast(JoystickButtonId::eJoystickButton9),
  eXboxRightStickButton = Core::enumCast(JoystickButtonId::eJoystickButton10),
  eXboxPadUp            = Core::enumCast(JoystickButtonId::eJoystickButton11),
  eXboxPadRight         = Core::enumCast(JoystickButtonId::eJoystickButton12),
  eXboxPadDown          = Core::enumCast(JoystickButtonId::eJoystickButton13),
  eXboxPadLeft          = Core::enumCast(JoystickButtonId::eJoystickButton14)
};

enum class Xbox360ControllerAxis : uint16_t {
  eXboxLeftStickX   = Core::enumCast(JoystickAxisId::eJoystickAxis0),
  eXboxLeftStickY   = Core::enumCast(JoystickAxisId::eJoystickAxis1),
  eXboxLeftTrigger  = Core::enumCast(JoystickAxisId::eJoystickAxis2),
  eXboxRightStickX  = Core::enumCast(JoystickAxisId::eJoystickAxis3),
  eXboxRightStickY  = Core::enumCast(JoystickAxisId::eJoystickAxis4),
  eXboxRightTrigger = Core::enumCast(JoystickAxisId::eJoystickAxis5)
};

enum class Button : uint16_t {
  eNone    = 0,
  eButton1 = 1,
  eButton2 = 2,
  eButton3 = 3,
  eButton4 = 4,
  eButton5 = 5,
  eButton6 = 6,
  eButton7 = 7,
  eButton8 = 8
};

enum class Modifier : uint16_t {
  eNone         = 0,
  eCapsLock     = 1 << 0,
  eShift        = 1 << 1,
  eControl      = 1 << 2,
  eAlt          = 1 << 3,
  eLeftButton   = 1 << 4,
  eMiddleButton = 1 << 5,
  eRightButton  = 1 << 6,
  eCommand      = 1 << 7,
  eNumLock      = 1 << 8,
  eIsKeyPad     = 1 << 9,
  eIsLeft       = 1 << 10,
  eIsRight      = 1 << 11
};

// codes based on the chromium keys at
// https://github.com/adobe/webkit/blob/master/Source/WebCore/platform/WindowsKeyboardCodes.h
enum class Key : uint32_t {
  eUnknown                = 0,
  eBackspace              = 0x08,
  eTab                    = 0x09,
  eClear                  = 0x0C,
  eReturn                 = 0x0D,
  eShift                  = 0x10,
  eControl                = 0x11,
  eAlt                    = 0x12,
  ePause                  = 0x13,
  eCapsLock               = 0x14,
  eKana                   = 0x15,
  eJunja                  = 0x17,
  eFinal                  = 0x18,
  eHanja                  = 0x19,
  eEscape                 = 0x1B,
  eConvert                = 0x1C,
  eNonconvert             = 0x1D,
  eAccept                 = 0x1E,
  eModechange             = 0x1F,
  eSpace                  = 0x20,
  ePageUp                 = 0x21,
  ePageDown               = 0x22,
  eEnd                    = 0x23,
  eHome                   = 0x24,
  eLeft                   = 0x25,
  eUp                     = 0x26,
  eRight                  = 0x27,
  eDown                   = 0x28,
  eSelect                 = 0x29,
  ePrint                  = 0x2A,
  eExecute                = 0x2B,
  ePrintScreen            = 0x2C,
  eInsert                 = 0x2D,
  eDelete                 = 0x2E,
  eHelp                   = 0x2F,
  e0                      = 0x30,
  e1                      = 0x31,
  e2                      = 0x32,
  e3                      = 0x33,
  e4                      = 0x34,
  e5                      = 0x35,
  e6                      = 0x36,
  e7                      = 0x37,
  e8                      = 0x38,
  e9                      = 0x39,
  eA                      = 0x41,
  eB                      = 0x42,
  eC                      = 0x43,
  eD                      = 0x44,
  eE                      = 0x45,
  eF                      = 0x46,
  eG                      = 0x47,
  eH                      = 0x48,
  eI                      = 0x49,
  eJ                      = 0x4A,
  eK                      = 0x4B,
  eL                      = 0x4C,
  eM                      = 0x4D,
  eN                      = 0x4E,
  eO                      = 0x4F,
  eP                      = 0x50,
  eQ                      = 0x51,
  eR                      = 0x52,
  eS                      = 0x53,
  eT                      = 0x54,
  eU                      = 0x55,
  eV                      = 0x56,
  eW                      = 0x57,
  eX                      = 0x58,
  eY                      = 0x59,
  eZ                      = 0x5A,
  eLeftSuper              = 0x5B,
  eRightSuper             = 0x5C,
  eApps                   = 0x5D,
  eSleep                  = 0x5F,
  eKp0                    = 0x60,
  eKp1                    = 0x61,
  eKp2                    = 0x62,
  eKp3                    = 0x63,
  eKp4                    = 0x64,
  eKp5                    = 0x65,
  eKp6                    = 0x66,
  eKp7                    = 0x67,
  eKp8                    = 0x68,
  eKp9                    = 0x69,
  eKpMultiply             = 0x6A,
  eKpAdd                  = 0x6B,
  eKpSeparator            = 0x6C,
  eKpSubtract             = 0x6D,
  eKpDecimal              = 0x6E,
  eKpDivide               = 0x6F,
  eF1                     = 0x70,
  eF2                     = 0x71,
  eF3                     = 0x72,
  eF4                     = 0x73,
  eF5                     = 0x74,
  eF6                     = 0x75,
  eF7                     = 0x76,
  eF8                     = 0x77,
  eF9                     = 0x78,
  eF10                    = 0x79,
  eF11                    = 0x7A,
  eF12                    = 0x7B,
  eF13                    = 0x7C,
  eF14                    = 0x7D,
  eF15                    = 0x7E,
  eF16                    = 0x7F,
  eF17                    = 0x80,
  eF18                    = 0x81,
  eF19                    = 0x82,
  eF20                    = 0x83,
  eF21                    = 0x84,
  eF22                    = 0x85,
  eF23                    = 0x86,
  eF24                    = 0x87,
  eNumLock                = 0x90,
  eScrollLock             = 0x91,
  eLeftShift              = 0xA0,
  eRightShift             = 0xA1,
  eLeftControl            = 0xA2,
  eRightControl           = 0xA3,
  eLeftMenu               = 0xA4,
  eRightMenu              = 0xA5,
  eBrowserBack            = 0xA6,
  eBrowserForward         = 0xA7,
  eBrowserRefresh         = 0xA8,
  eBrowserStop            = 0xA9,
  eBrowserSearch          = 0xAA,
  eBrowserFavorites       = 0xAB,
  eBrowserHome            = 0xAC,
  eVolumeMute             = 0xAD,
  eVolumeDown             = 0xAE,
  eVolumeUp               = 0xAF,
  eMediaNextTrack         = 0xB0,
  eMediaPrevTrack         = 0xB1,
  eMediaStop              = 0xB2,
  eMediaPlayPause         = 0xB3,
  eMediaLaunchMail        = 0xB4,
  eMediaLaunchMediaSelect = 0xB5,
  eMediaLaunchApp1        = 0xB6,
  eMediaLaunchApp2        = 0xB7,
  ePlus                   = 0xBB,
  eComma                  = 0xBC,
  eMinus                  = 0xBD,
  ePeriod                 = 0xBE,
  eOem1                   = 0xBA, // The ';:' key
  eOem2                   = 0xBF, // The '/?' key
  eOem3                   = 0xC0, // The '`~' key
  eOem4                   = 0xDB, // The '[{' key
  eOem5                   = 0xDC, // The '\|' key
  eOem6                   = 0xDD, // The ']}' key
  eOem7                   = 0xDE, // The 'single-quote/double-quote' key
  eOem8                   = 0xDF, // Used for miscellaneous characters; it can vary by keyboard.
  eOem102 = 0xE2 // Either the angle bracket key or the backslash key on the RT 102-key keyboard
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
