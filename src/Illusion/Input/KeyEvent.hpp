////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_INPUT_KEY_EVENT_HPP
#define ILLUSION_INPUT_KEY_EVENT_HPP

#include "Enums.hpp"

namespace Illusion::Input {

struct KeyEvent {

  enum class Type { ePress, eRelease, eRepeat, eCharacter };

  KeyEvent();
  KeyEvent(int32_t key, int32_t scancode, int32_t action, int32_t mods);
  KeyEvent(uint32_t key, int32_t mods);
  void SetMods(int32_t mods);

  Type mType;

  // bitwise or of any Modifier defined in InputEnums.hpp
  uint32_t mModifiers;

  int32_t mScancode;

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
