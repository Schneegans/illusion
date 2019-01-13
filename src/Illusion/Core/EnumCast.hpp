////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
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
