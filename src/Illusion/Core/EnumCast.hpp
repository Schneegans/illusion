////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_ENUM_CAST_HPP
#define ILLUSION_CORE_ENUM_CAST_HPP

#include <type_traits>

namespace Illusion::Core {

// A template to cast an enum class to its underlying type.
template <typename T>
constexpr typename std::underlying_type<T>::type enumCast(T val) {
  return static_cast<typename std::underlying_type<T>::type>(val);
}

} // namespace Illusion::Core

#endif // ILLUSION_CORE_ENUM_CAST_HPP
