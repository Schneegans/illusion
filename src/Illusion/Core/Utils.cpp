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
  if (s.empty()) {
    return {""};
  }

  std::vector<std::string> elems;

  std::stringstream ss(s);
  std::string       item;

  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }

  return elems;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string joinStrings(std::vector<std::string> const& parts, std::string const& delim) {
  std::string result;

  for (size_t i(0); i < parts.size(); ++i) {
    result += parts[i];

    if (i + 1 < parts.size()) {
      result += delim;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string joinStrings(
    std::vector<std::string> const& parts, std::string const& delim, std::string const& lastDelim) {
  std::string result;

  for (size_t i(0); i < parts.size(); ++i) {
    result += parts[i];

    if (i + 2 == parts.size()) {
      result += lastDelim;
    } else if (i + 1 < parts.size()) {
      result += delim;
    }
  }

  return result;
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
