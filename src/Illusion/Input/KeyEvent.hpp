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
  KeyEvent(int32_t key, int32_t scancode, int32_t action, uint32_t mods);
  KeyEvent(uint32_t key, uint32_t mods);
  void SetMods(uint32_t mods);

  Type mType;

  // bitwise or of any Modifier defined in InputEnums.hpp
  uint16_t mModifiers = 0;
  int32_t  mScancode  = 0;

  // only used for ePress, eRelease and eRepeat
  Key mKey = Key::eUnknown;

  // only used for eCharacter
  uint16_t mCharacter = 0;
};

std::ostream& operator<<(std::ostream& os, KeyEvent const& e);

} // namespace Illusion::Input

#endif // ILLUSION_INPUT_KEY_EVENT_HPP
