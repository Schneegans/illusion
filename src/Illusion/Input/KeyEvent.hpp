////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_INPUT_KEY_EVENT_HPP
#define ILLUSION_INPUT_KEY_EVENT_HPP

#include "Enums.hpp"

namespace Illusion::Input {

struct KeyEvent {

  enum class Type { ePress, eRelease, eRepeat, eCharacter };

  KeyEvent();
  KeyEvent(int key, int scancode, int action, int mods);
  KeyEvent(unsigned int key, int mods);
  void SetMods(int mods);

  Type mType;

  // bitwise or of any Modifier defined in InputEnums.hpp
  uint32_t mModifiers;

  int mScancode;

  union {
    // only used for ePress, eRelease and eRepeat
    Key mKey;

    // only used for eCharacter
    uint16_t mCharacter;
  };
};

std::ostream& operator<<(std::ostream& os, KeyEvent const& e);

} // namespace Illusion::Input

#endif // ILLUSION_INPUT_KEY_EVENT_HPP
