////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KeyEvent.hpp"

#include <GLFW/glfw3.h>

namespace Illusion::Input {

////////////////////////////////////////////////////////////////////////////////////////////////////

KeyEvent::KeyEvent()
    : mType(Type::ePress) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KeyEvent::KeyEvent(int32_t key, int32_t scancode, int32_t action, uint32_t mods) {
  if (action == GLFW_RELEASE) {
    mType = KeyEvent::Type::eRelease;
  } else if (action == GLFW_REPEAT) {
    mType = KeyEvent::Type::eRepeat;
  } else {
    mType = KeyEvent::Type::ePress;
  }

  mScancode = scancode;

  // clang-format off
  switch (key) {
    case GLFW_KEY_SPACE:         mKey = Key::eSpace;        break;
    // case GLFW_KEY_APOSTROPHE:    mKey = Key::eApostrophe;   break;
    case GLFW_KEY_COMMA:         mKey = Key::eComma;        break;
    case GLFW_KEY_MINUS:         mKey = Key::eMinus;        break;
    case GLFW_KEY_PERIOD:        mKey = Key::ePeriod;       break;
    case GLFW_KEY_SLASH:         mKey = Key::eOem2;         break;
    case GLFW_KEY_0:             mKey = Key::e0;            break;
    case GLFW_KEY_1:             mKey = Key::e1;            break;
    case GLFW_KEY_2:             mKey = Key::e2;            break;
    case GLFW_KEY_3:             mKey = Key::e3;            break;
    case GLFW_KEY_4:             mKey = Key::e4;            break;
    case GLFW_KEY_5:             mKey = Key::e5;            break;
    case GLFW_KEY_6:             mKey = Key::e6;            break;
    case GLFW_KEY_7:             mKey = Key::e7;            break;
    case GLFW_KEY_8:             mKey = Key::e8;            break;
    case GLFW_KEY_9:             mKey = Key::e9;            break;
    case GLFW_KEY_SEMICOLON:     mKey = Key::eOem1;         break;
    // case GLFW_KEY_EQUAL:         mKey = Key::eEqual;        break;
    case GLFW_KEY_A:             mKey = Key::eA;            break;
    case GLFW_KEY_B:             mKey = Key::eB;            break;
    case GLFW_KEY_C:             mKey = Key::eC;            break;
    case GLFW_KEY_D:             mKey = Key::eD;            break;
    case GLFW_KEY_E:             mKey = Key::eE;            break;
    case GLFW_KEY_F:             mKey = Key::eF;            break;
    case GLFW_KEY_G:             mKey = Key::eG;            break;
    case GLFW_KEY_H:             mKey = Key::eH;            break;
    case GLFW_KEY_I:             mKey = Key::eI;            break;
    case GLFW_KEY_J:             mKey = Key::eJ;            break;
    case GLFW_KEY_K:             mKey = Key::eK;            break;
    case GLFW_KEY_L:             mKey = Key::eL;            break;
    case GLFW_KEY_M:             mKey = Key::eM;            break;
    case GLFW_KEY_N:             mKey = Key::eN;            break;
    case GLFW_KEY_O:             mKey = Key::eO;            break;
    case GLFW_KEY_P:             mKey = Key::eP;            break;
    case GLFW_KEY_Q:             mKey = Key::eQ;            break;
    case GLFW_KEY_R:             mKey = Key::eR;            break;
    case GLFW_KEY_S:             mKey = Key::eS;            break;
    case GLFW_KEY_T:             mKey = Key::eT;            break;
    case GLFW_KEY_U:             mKey = Key::eU;            break;
    case GLFW_KEY_V:             mKey = Key::eV;            break;
    case GLFW_KEY_W:             mKey = Key::eW;            break;
    case GLFW_KEY_X:             mKey = Key::eX;            break;
    case GLFW_KEY_Y:             mKey = Key::eY;            break;
    case GLFW_KEY_Z:             mKey = Key::eZ;            break;
    case GLFW_KEY_LEFT_BRACKET:  mKey = Key::eOem4;         break;
    case GLFW_KEY_BACKSLASH:     mKey = Key::eOem5;         break;
    case GLFW_KEY_RIGHT_BRACKET: mKey = Key::eOem6;         break;
    // case GLFW_KEY_GRAVE_ACCENT:  mKey = Key::eGrave_accent; break;
    // case GLFW_KEY_WORLD_1:       mKey = Key::eWorld_1;      break;
    // case GLFW_KEY_WORLD_2:       mKey = Key::eWorld_2;      break;
    case GLFW_KEY_ESCAPE:        mKey = Key::eEscape;       break;
    case GLFW_KEY_ENTER:         mKey = Key::eReturn;       break;
    case GLFW_KEY_TAB:           mKey = Key::eTab;          break;
    case GLFW_KEY_BACKSPACE:     mKey = Key::eBackspace;    break;
    case GLFW_KEY_INSERT:        mKey = Key::eInsert;       break;
    case GLFW_KEY_DELETE:        mKey = Key::eDelete;       break;
    case GLFW_KEY_RIGHT:         mKey = Key::eRight;        break;
    case GLFW_KEY_LEFT:          mKey = Key::eLeft;         break;
    case GLFW_KEY_DOWN:          mKey = Key::eDown;         break;
    case GLFW_KEY_UP:            mKey = Key::eUp;           break;
    case GLFW_KEY_PAGE_UP:       mKey = Key::ePageUp;       break;
    case GLFW_KEY_PAGE_DOWN:     mKey = Key::ePageDown;     break;
    case GLFW_KEY_HOME:          mKey = Key::eHome;         break;
    case GLFW_KEY_END:           mKey = Key::eEnd;          break;
    case GLFW_KEY_CAPS_LOCK:     mKey = Key::eCapsLock;     break;
    case GLFW_KEY_SCROLL_LOCK:   mKey = Key::eScrollLock;   break;
    case GLFW_KEY_NUM_LOCK:      mKey = Key::eNumLock;      break;
    case GLFW_KEY_PRINT_SCREEN:  mKey = Key::ePrintScreen;  break;
    case GLFW_KEY_PAUSE:         mKey = Key::ePause;        break;
    case GLFW_KEY_F1:            mKey = Key::eF1;           break;
    case GLFW_KEY_F2:            mKey = Key::eF2;           break;
    case GLFW_KEY_F3:            mKey = Key::eF3;           break;
    case GLFW_KEY_F4:            mKey = Key::eF4;           break;
    case GLFW_KEY_F5:            mKey = Key::eF5;           break;
    case GLFW_KEY_F6:            mKey = Key::eF6;           break;
    case GLFW_KEY_F7:            mKey = Key::eF7;           break;
    case GLFW_KEY_F8:            mKey = Key::eF8;           break;
    case GLFW_KEY_F9:            mKey = Key::eF9;           break;
    case GLFW_KEY_F10:           mKey = Key::eF10;          break;
    case GLFW_KEY_F11:           mKey = Key::eF11;          break;
    case GLFW_KEY_F12:           mKey = Key::eF12;          break;
    case GLFW_KEY_F13:           mKey = Key::eF13;          break;
    case GLFW_KEY_F14:           mKey = Key::eF14;          break;
    case GLFW_KEY_F15:           mKey = Key::eF15;          break;
    case GLFW_KEY_F16:           mKey = Key::eF16;          break;
    case GLFW_KEY_F17:           mKey = Key::eF17;          break;
    case GLFW_KEY_F18:           mKey = Key::eF18;          break;
    case GLFW_KEY_F19:           mKey = Key::eF19;          break;
    case GLFW_KEY_F20:           mKey = Key::eF20;          break;
    case GLFW_KEY_F21:           mKey = Key::eF21;          break;
    case GLFW_KEY_F22:           mKey = Key::eF22;          break;
    case GLFW_KEY_F23:           mKey = Key::eF23;          break;
    case GLFW_KEY_F24:           mKey = Key::eF24;          break;
    case GLFW_KEY_KP_0:          mKey = Key::eKp0;          break;
    case GLFW_KEY_KP_1:          mKey = Key::eKp1;          break;
    case GLFW_KEY_KP_2:          mKey = Key::eKp2;          break;
    case GLFW_KEY_KP_3:          mKey = Key::eKp3;          break;
    case GLFW_KEY_KP_4:          mKey = Key::eKp4;          break;
    case GLFW_KEY_KP_5:          mKey = Key::eKp5;          break;
    case GLFW_KEY_KP_6:          mKey = Key::eKp6;          break;
    case GLFW_KEY_KP_7:          mKey = Key::eKp7;          break;
    case GLFW_KEY_KP_8:          mKey = Key::eKp8;          break;
    case GLFW_KEY_KP_9:          mKey = Key::eKp9;          break;
    case GLFW_KEY_KP_DECIMAL:    mKey = Key::eKpDecimal;    break;
    case GLFW_KEY_KP_DIVIDE:     mKey = Key::eKpDivide;     break;
    case GLFW_KEY_KP_MULTIPLY:   mKey = Key::eKpMultiply;   break;
    case GLFW_KEY_KP_SUBTRACT:   mKey = Key::eKpSubtract;   break;
    case GLFW_KEY_KP_ADD:        mKey = Key::eKpAdd;        break;
    // case GLFW_KEY_KP_ENTER:      mKey = Key::eKpEnter;      break;
    // case GLFW_KEY_KP_EQUAL:      mKey = Key::eKpEqual;      break;
    case GLFW_KEY_LEFT_SHIFT:    mKey = Key::eLeftShift;    break;
    case GLFW_KEY_LEFT_CONTROL:  mKey = Key::eLeftControl;  break;
    case GLFW_KEY_LEFT_ALT:      mKey = Key::eAlt;          break;
    case GLFW_KEY_LEFT_SUPER:    mKey = Key::eLeftSuper;    break;
    case GLFW_KEY_RIGHT_SHIFT:   mKey = Key::eRightShift;   break;
    case GLFW_KEY_RIGHT_CONTROL: mKey = Key::eRightControl; break;
    case GLFW_KEY_RIGHT_ALT:     mKey = Key::eAlt;          break;
    case GLFW_KEY_RIGHT_SUPER:   mKey = Key::eRightSuper;   break;
    case GLFW_KEY_MENU:          mKey = Key::eLeftMenu;     break;
    default:                     mKey = Key::eUnknown;      break;
  }
  // clang-format on

  SetMods(mods);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KeyEvent::KeyEvent(uint32_t key, uint32_t mods) {
  mType      = KeyEvent::Type::eCharacter;
  mCharacter = key;
  SetMods(mods);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void KeyEvent::SetMods(uint32_t mods) {
  if ((mods & static_cast<uint32_t>(GLFW_MOD_SHIFT)) != 0) {
    mModifiers |= Core::enumCast(Modifier::eShift);
  }
  if ((mods & static_cast<uint32_t>(GLFW_MOD_CONTROL)) != 0) {
    mModifiers |= Core::enumCast(Modifier::eControl);
  }
  if ((mods & static_cast<uint32_t>(GLFW_MOD_ALT)) != 0) {
    mModifiers |= Core::enumCast(Modifier::eAlt);
  }
  if ((mods & static_cast<uint32_t>(GLFW_MOD_SUPER)) != 0) {
    mModifiers |= Core::enumCast(Modifier::eCommand);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, KeyEvent const& e) {
  switch (e.mType) {
    case KeyEvent::Type::ePress:
      os << "PRESS " << e.mKey << " " << e.mScancode << " " << e.mModifiers;
      return os;
    case KeyEvent::Type::eRelease:
      os << "RELEASE " << e.mKey << " " << e.mScancode << " " << e.mModifiers;
      return os;
    case KeyEvent::Type::eRepeat:
      os << "REPEAT " << e.mKey << " " << e.mScancode << " " << e.mModifiers;
      return os;
    case KeyEvent::Type::eCharacter:
      os << "CHARACTER " << e.mCharacter << " " << e.mScancode << " " << e.mModifiers;
      return os;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Input
