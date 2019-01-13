////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "filesystem.hpp"

#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#endif

namespace Illusion::Core::FileSystem {

time_t getLastWriteTime(std::string const& filename) {
#ifdef WIN32
  struct _stat result;
  if (_stat(filename.c_str(), &result) == 0) {
#else
  struct stat result;
  if (stat(filename.c_str(), &result) == 0) {
#endif
    return result.st_mtime;
  }
  return 0;
}

} // namespace Illusion::Core::FileSystem
