////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AnimatedProperty.hpp"

#include "EnumCast.hpp"

#include <iostream>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, AnimationDirection value) {
  os << enumCast(value);
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream& operator>>(std::istream& is, AnimationDirection& value) {
  std::underlying_type<AnimationDirection>::type tmp(0);
  is >> tmp;
  value = static_cast<AnimationDirection>(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, AnimationLoop value) {
  os << enumCast(value);
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream& operator>>(std::istream& is, AnimationLoop& value) {
  std::underlying_type<AnimationLoop>::type tmp(0);
  is >> tmp;
  value = static_cast<AnimationLoop>(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core
