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
#include "KeyEvent.hpp"

#include <GLFW/glfw3.h>

namespace Illusion::Input {

KeyEvent::KeyEvent()
  : mType(Type::PRESS)
  , mModifiers(0)
  , mScancode(0)
  , mKey(Key::UNKNOWN) {}

KeyEvent::KeyEvent(int key, int scancode, int action, int mods) {
  if (action == GLFW_RELEASE)
    mType = KeyEvent::Type::RELEASE;
  else if (action == GLFW_REPEAT)
    mType = KeyEvent::Type::REPEAT;
  else
    mType = KeyEvent::Type::PRESS;

  mScancode = scancode;

  switch (key) {
  case GLFW_KEY_SPACE:
    mKey = Key::SPACE;
    break;
  // case GLFW_KEY_APOSTROPHE:
  //   mKey = Key::APOSTROPHE;
  //   break;
  case GLFW_KEY_COMMA:
    mKey = Key::COMMA;
    break;
  case GLFW_KEY_MINUS:
    mKey = Key::MINUS;
    break;
  case GLFW_KEY_PERIOD:
    mKey = Key::PERIOD;
    break;
  case GLFW_KEY_SLASH:
    mKey = Key::OEM_2;
    break;
  case GLFW_KEY_0:
    mKey = Key::KEY_0;
    break;
  case GLFW_KEY_1:
    mKey = Key::KEY_1;
    break;
  case GLFW_KEY_2:
    mKey = Key::KEY_2;
    break;
  case GLFW_KEY_3:
    mKey = Key::KEY_3;
    break;
  case GLFW_KEY_4:
    mKey = Key::KEY_4;
    break;
  case GLFW_KEY_5:
    mKey = Key::KEY_5;
    break;
  case GLFW_KEY_6:
    mKey = Key::KEY_6;
    break;
  case GLFW_KEY_7:
    mKey = Key::KEY_7;
    break;
  case GLFW_KEY_8:
    mKey = Key::KEY_8;
    break;
  case GLFW_KEY_9:
    mKey = Key::KEY_9;
    break;
  case GLFW_KEY_SEMICOLON:
    mKey = Key::OEM_1;
    break;
  // case GLFW_KEY_EQUAL:
  //   mKey = Key::EQUAL;
  //   break;
  case GLFW_KEY_A:
    mKey = Key::KEY_A;
    break;
  case GLFW_KEY_B:
    mKey = Key::KEY_B;
    break;
  case GLFW_KEY_C:
    mKey = Key::KEY_C;
    break;
  case GLFW_KEY_D:
    mKey = Key::KEY_D;
    break;
  case GLFW_KEY_E:
    mKey = Key::KEY_E;
    break;
  case GLFW_KEY_F:
    mKey = Key::KEY_F;
    break;
  case GLFW_KEY_G:
    mKey = Key::KEY_G;
    break;
  case GLFW_KEY_H:
    mKey = Key::KEY_H;
    break;
  case GLFW_KEY_I:
    mKey = Key::KEY_I;
    break;
  case GLFW_KEY_J:
    mKey = Key::KEY_J;
    break;
  case GLFW_KEY_K:
    mKey = Key::KEY_K;
    break;
  case GLFW_KEY_L:
    mKey = Key::KEY_L;
    break;
  case GLFW_KEY_M:
    mKey = Key::KEY_M;
    break;
  case GLFW_KEY_N:
    mKey = Key::KEY_N;
    break;
  case GLFW_KEY_O:
    mKey = Key::KEY_O;
    break;
  case GLFW_KEY_P:
    mKey = Key::KEY_P;
    break;
  case GLFW_KEY_Q:
    mKey = Key::KEY_Q;
    break;
  case GLFW_KEY_R:
    mKey = Key::KEY_R;
    break;
  case GLFW_KEY_S:
    mKey = Key::KEY_S;
    break;
  case GLFW_KEY_T:
    mKey = Key::KEY_T;
    break;
  case GLFW_KEY_U:
    mKey = Key::KEY_U;
    break;
  case GLFW_KEY_V:
    mKey = Key::KEY_V;
    break;
  case GLFW_KEY_W:
    mKey = Key::KEY_W;
    break;
  case GLFW_KEY_X:
    mKey = Key::KEY_X;
    break;
  case GLFW_KEY_Y:
    mKey = Key::KEY_Y;
    break;
  case GLFW_KEY_Z:
    mKey = Key::KEY_Z;
    break;
  case GLFW_KEY_LEFT_BRACKET:
    mKey = Key::OEM_4;
    break;
  case GLFW_KEY_BACKSLASH:
    mKey = Key::OEM_5;
    break;
  case GLFW_KEY_RIGHT_BRACKET:
    mKey = Key::OEM_6;
    break;
  // case GLFW_KEY_GRAVE_ACCENT:
  //   mKey = Key::GRAVE_ACCENT;
  //   break;
  // case GLFW_KEY_WORLD_1:
  //   mKey = Key::WORLD_1;
  //   break;
  // case GLFW_KEY_WORLD_2:
  //   mKey = Key::WORLD_2;
  //   break;
  case GLFW_KEY_ESCAPE:
    mKey = Key::ESCAPE;
    break;
  case GLFW_KEY_ENTER:
    mKey = Key::RETURN;
    break;
  case GLFW_KEY_TAB:
    mKey = Key::TAB;
    break;
  case GLFW_KEY_BACKSPACE:
    mKey = Key::BACKSPACE;
    break;
  case GLFW_KEY_INSERT:
    mKey = Key::INSERT;
    break;
  case GLFW_KEY_DELETE:
    mKey = Key::KEY_DELETE;
    break;
  case GLFW_KEY_RIGHT:
    mKey = Key::RIGHT;
    break;
  case GLFW_KEY_LEFT:
    mKey = Key::LEFT;
    break;
  case GLFW_KEY_DOWN:
    mKey = Key::DOWN;
    break;
  case GLFW_KEY_UP:
    mKey = Key::UP;
    break;
  case GLFW_KEY_PAGE_UP:
    mKey = Key::PAGE_UP;
    break;
  case GLFW_KEY_PAGE_DOWN:
    mKey = Key::PAGE_DOWN;
    break;
  case GLFW_KEY_HOME:
    mKey = Key::HOME;
    break;
  case GLFW_KEY_END:
    mKey = Key::END;
    break;
  case GLFW_KEY_CAPS_LOCK:
    mKey = Key::CAPS_LOCK;
    break;
  case GLFW_KEY_SCROLL_LOCK:
    mKey = Key::SCROLL_LOCK;
    break;
  case GLFW_KEY_NUM_LOCK:
    mKey = Key::NUM_LOCK;
    break;
  case GLFW_KEY_PRINT_SCREEN:
    mKey = Key::PRINT_SCREEN;
    break;
  case GLFW_KEY_PAUSE:
    mKey = Key::PAUSE;
    break;
  case GLFW_KEY_F1:
    mKey = Key::F1;
    break;
  case GLFW_KEY_F2:
    mKey = Key::F2;
    break;
  case GLFW_KEY_F3:
    mKey = Key::F3;
    break;
  case GLFW_KEY_F4:
    mKey = Key::F4;
    break;
  case GLFW_KEY_F5:
    mKey = Key::F5;
    break;
  case GLFW_KEY_F6:
    mKey = Key::F6;
    break;
  case GLFW_KEY_F7:
    mKey = Key::F7;
    break;
  case GLFW_KEY_F8:
    mKey = Key::F8;
    break;
  case GLFW_KEY_F9:
    mKey = Key::F9;
    break;
  case GLFW_KEY_F10:
    mKey = Key::F10;
    break;
  case GLFW_KEY_F11:
    mKey = Key::F11;
    break;
  case GLFW_KEY_F12:
    mKey = Key::F12;
    break;
  case GLFW_KEY_F13:
    mKey = Key::F13;
    break;
  case GLFW_KEY_F14:
    mKey = Key::F14;
    break;
  case GLFW_KEY_F15:
    mKey = Key::F15;
    break;
  case GLFW_KEY_F16:
    mKey = Key::F16;
    break;
  case GLFW_KEY_F17:
    mKey = Key::F17;
    break;
  case GLFW_KEY_F18:
    mKey = Key::F18;
    break;
  case GLFW_KEY_F19:
    mKey = Key::F19;
    break;
  case GLFW_KEY_F20:
    mKey = Key::F20;
    break;
  case GLFW_KEY_F21:
    mKey = Key::F21;
    break;
  case GLFW_KEY_F22:
    mKey = Key::F22;
    break;
  case GLFW_KEY_F23:
    mKey = Key::F23;
    break;
  case GLFW_KEY_F24:
    mKey = Key::F24;
    break;
  case GLFW_KEY_KP_0:
    mKey = Key::KP_0;
    break;
  case GLFW_KEY_KP_1:
    mKey = Key::KP_1;
    break;
  case GLFW_KEY_KP_2:
    mKey = Key::KP_2;
    break;
  case GLFW_KEY_KP_3:
    mKey = Key::KP_3;
    break;
  case GLFW_KEY_KP_4:
    mKey = Key::KP_4;
    break;
  case GLFW_KEY_KP_5:
    mKey = Key::KP_5;
    break;
  case GLFW_KEY_KP_6:
    mKey = Key::KP_6;
    break;
  case GLFW_KEY_KP_7:
    mKey = Key::KP_7;
    break;
  case GLFW_KEY_KP_8:
    mKey = Key::KP_8;
    break;
  case GLFW_KEY_KP_9:
    mKey = Key::KP_9;
    break;
  case GLFW_KEY_KP_DECIMAL:
    mKey = Key::KP_DECIMAL;
    break;
  case GLFW_KEY_KP_DIVIDE:
    mKey = Key::KP_DIVIDE;
    break;
  case GLFW_KEY_KP_MULTIPLY:
    mKey = Key::KP_MULTIPLY;
    break;
  case GLFW_KEY_KP_SUBTRACT:
    mKey = Key::KP_SUBTRACT;
    break;
  case GLFW_KEY_KP_ADD:
    mKey = Key::KP_ADD;
    break;
  // case GLFW_KEY_KP_ENTER:
  //   mKey = Key::KP_ENTER;
  //   break;
  // case GLFW_KEY_KP_EQUAL:
  //   mKey = Key::KP_EQUAL;
  //   break;
  case GLFW_KEY_LEFT_SHIFT:
    mKey = Key::LEFT_SHIFT;
    break;
  case GLFW_KEY_LEFT_CONTROL:
    mKey = Key::LEFT_CONTROL;
    break;
  case GLFW_KEY_LEFT_ALT:
    mKey = Key::ALT;
    break;
  case GLFW_KEY_LEFT_SUPER:
    mKey = Key::LEFT_SUPER;
    break;
  case GLFW_KEY_RIGHT_SHIFT:
    mKey = Key::RIGHT_SHIFT;
    break;
  case GLFW_KEY_RIGHT_CONTROL:
    mKey = Key::RIGHT_CONTROL;
    break;
  case GLFW_KEY_RIGHT_ALT:
    mKey = Key::ALT;
    break;
  case GLFW_KEY_RIGHT_SUPER:
    mKey = Key::RIGHT_SUPER;
    break;
  case GLFW_KEY_MENU:
    mKey = Key::LEFT_MENU;
    break;
  default:
    mKey = Key::UNKNOWN;
  }

  SetMods(mods);
}

KeyEvent::KeyEvent(unsigned int key, int mods) {
  mType      = KeyEvent::Type::CHARACTER;
  mCharacter = key;
  SetMods(mods);
}

void KeyEvent::SetMods(int mods) {
  if (mods & GLFW_MOD_SHIFT) mModifiers |= (int)Modifier::SHIFT;
  if (mods & GLFW_MOD_CONTROL) mModifiers |= (int)Modifier::CONTROL;
  if (mods & GLFW_MOD_ALT) mModifiers |= (int)Modifier::ALT;
  if (mods & GLFW_MOD_SUPER) mModifiers |= (int)Modifier::COMMAND;
}

std::ostream& operator<<(std::ostream& os, KeyEvent const& e) {
  switch (e.mType) {
  case KeyEvent::Type::PRESS:
    os << "PRESS " << e.mKey << " " << e.mScancode << " " << e.mModifiers;
    return os;
  case KeyEvent::Type::RELEASE:
    os << "RELEASE " << e.mKey << " " << e.mScancode << " " << e.mModifiers;
    return os;
  case KeyEvent::Type::REPEAT:
    os << "REPEAT " << e.mKey << " " << e.mScancode << " " << e.mModifiers;
    return os;
  case KeyEvent::Type::CHARACTER:
    os << "CHARACTER " << e.mCharacter << " " << e.mScancode << " " << e.mModifiers;
    return os;
  }
  return os;
}

} // namespace Illusion::Input
