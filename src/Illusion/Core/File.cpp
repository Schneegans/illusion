////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "File.hpp"

#include "filesystem.hpp"

#include <sstream>
#include <vector>

namespace Illusion::Core {

File::File(std::string const& fileName)
    : mPath(fileName) {

  resetChangedOnDisc();
}

bool File::isValid() const {
  std::ifstream file(mPath.c_str());
  return !file.fail();
}

void File::remove() {
  std::remove(mPath.c_str());
}

std::string const& File::getFileName() const {
  return mPath;
}

void File::setFileName(std::string const& path) {
  mPath = path;
}

time_t File::getLastWriteTime() const {
  return FileSystem::getLastWriteTime(mPath);
}

bool File::changedOnDisc() const {
  auto time = getLastWriteTime();
  return time != mLastWriteTime;
}

void File::resetChangedOnDisc() {
  mLastWriteTime = getLastWriteTime();
}

} // namespace Illusion::Core
