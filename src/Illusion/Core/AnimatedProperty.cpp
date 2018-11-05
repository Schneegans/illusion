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
#include "AnimatedProperty.hpp"

#include <iostream>

namespace Illusion::Core {

std::ostream& operator<<(std::ostream& os, AnimationDirection value) {
  os << static_cast<int>(value);
  return os;
}

std::istream& operator>>(std::istream& is, AnimationDirection& value) {
  int tmp(0);
  is >> tmp;
  value = static_cast<AnimationDirection>(tmp);
  return is;
}

std::ostream& operator<<(std::ostream& os, AnimationLoop value) {
  os << static_cast<int>(value);
  return os;
}

std::istream& operator>>(std::istream& is, AnimationLoop& value) {
  int tmp(0);
  is >> tmp;
  value = static_cast<AnimationLoop>(tmp);
  return is;
}
} // namespace Illusion::Core
