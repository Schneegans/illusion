////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Utils.hpp"

#include <sstream>

namespace Illusion::Core::Utils {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> splitString(std::string const& s, char delim) {
  std::vector<std::string> elems;

  std::stringstream ss(s);
  std::string       item;

  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }

  return elems;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool stringContains(std::string const& s, char c) {
  return s.find(c) != std::string::npos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t replaceString(std::string& s, std::string const& oldString, std::string const& newString) {
  size_t   searchPos  = 0;
  uint32_t occurences = 0;
  while ((searchPos = s.find(oldString, searchPos)) != std::string::npos) {
    s.replace(searchPos, oldString.length(), newString);
    searchPos += newString.length();
    ++occurences;
  }
  return occurences;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core::Utils
