////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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
