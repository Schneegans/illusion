////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_INPUT_KEY_EVENT_HPP
#define ILLUSION_INPUT_KEY_EVENT_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Enums.hpp"

namespace Illusion::Input {

struct KeyEvent {

  enum class Type { PRESS, RELEASE, REPEAT, CHARACTER };

  // -------------------------------------------------------------------------------- public methods
  KeyEvent();
  KeyEvent(int key, int scancode, int action, int mods);
  KeyEvent(unsigned int key, int mods);
  void SetMods(int mods);

  // -------------------------------------------------------------------------------- public members
  // PRESS, RELEASE, CHARACTER
  Type mType;

  // bitwise or of any Modifier defined in InputEnums.hpp
  uint32_t mModifiers;

  int mScancode;

  union {
    // only used for PRESS, RELEASE and REPEAT
    Key mKey;

    // only used for CHARACTER
    uint16_t mCharacter;
  };
};

std::ostream& operator<<(std::ostream& os, KeyEvent const& e);

} // namespace Illusion::Input

#endif // ILLUSION_INPUT_KEY_EVENT_HPP
