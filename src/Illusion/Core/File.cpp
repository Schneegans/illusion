////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "File.hpp"

#include "filesystem.hpp"

#include <sstream>
#include <utility>
#include <vector>

namespace Illusion::Core {

File::File(std::string fileName)
    : mPath(std::move(fileName)) {

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
