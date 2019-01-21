////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_UTILS_HPP
#define ILLUSION_CORE_UTILS_HPP

#include <memory>
#include <vector>

namespace Illusion::Core::Utils {

// -------------------------------------------------------------------------------------------------

// Similar to std::make_shared(), this can be used to create a std::unique_ptr.
template <typename T, typename... Args>
std::unique_ptr<T> makeUnique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// -------------------------------------------------------------------------------------------------

// Splits the given string into parts, using the given delimiter.
std::vector<std::string> splitString(std::string const& s, char delim);

// Returns true when the given string contains the character c.
bool stringContains(std::string const& s, char c);

// Places all occurences of oldString with newString in s. Returns the number of occurences.
uint32_t replaceString(std::string& s, std::string const& oldString, std::string const& newString);

// -------------------------------------------------------------------------------------------------

} // namespace Illusion::Core::Utils

#endif // ILLUSION_CORE_UTILS_HPP
