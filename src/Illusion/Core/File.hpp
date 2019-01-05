////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_FILE_HPP
#define ILLUSION_CORE_FILE_HPP

#include "Logger.hpp"
#include "filesystem.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This class is used to read and write text files. The template parameter specifies the type the //
// content of the file is returned as. Common use cases include:                                  //
//                                                                                                //
// File<std::string>     mFile; // reads and writes the content as a std::string                  //
// File<vector<uint8_t>> mFile; // reads and writes the content as a byte array                   //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Illusion::Core {

template <typename T>
class File {

 public:
  // This constructs a invalid File.
  File()
      : mPath("")
      , mContent()
      , mIsLoaded(false) {
  }

  // This constructs a File for a given name.
  File(std::string const& fileName)
      : mPath(fileName)
      , mLastWriteTime(getLastWriteTime())
      , mContent()
      , mIsLoaded(false) {
  }

  // Returns if the given file is valid.
  bool isValid() const {
    std::ifstream file(mPath.c_str());
    return !file.fail();
  }

  // Returns the given file's content.
  T const& getContent() const {
    if (mIsLoaded) {
      return mContent;
    }

    std::ifstream ifs(mPath, std::ifstream::in | std::ios::binary);
    if (!ifs) {
      ILLUSION_WARNING << "Cannot open file \"" << mPath << "\"!" << std::endl;
      return mContent;
    }

    ifs.seekg(0, std::ios::end);
    size_t filesize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    mContent.resize(filesize / sizeof(typename T::value_type));

    ifs.read((char*)mContent.data(), filesize);
    ifs.close();

    return mContent;
  }

  // Sets the given file's content.
  void setContent(T const& content) {
    mContent  = content;
    mIsLoaded = true;
  }

  // Saves the file
  bool save() const {
    if (!mIsLoaded) {
      ILLUSION_WARNING << "Unable to save file \"" << mPath << "\"! No content has been set."
                       << std::endl;
      return false;
    }

    std::ofstream ofs(mPath, std::ifstream::out | std::ios::binary);
    if (!ofs) {
      ILLUSION_WARNING << "Cannot open file \"" << mPath << "\"!" << std::endl;
      return false;
    }

    ofs.write((char*)mContent.data(), mContent.size() * sizeof(typename T::value_type));
    ofs.close();

    return true;
  }

  // Deletes the file from the file system
  void remove() {
    std::remove(mPath.c_str());
    mContent.clear();
    mIsLoaded = false;
  }

  // Returns the given file's name.
  std::string const& getFileName() const {
    return mPath;
  }

  // Returns the last write time as reported from std::filesystem
  std::filesystem::file_time_type getLastWriteTime() const {
    return std::filesystem::last_write_time(mPath);
  }

  // Polls for changes to this file
  bool changedOnDisc() const {
    auto time = getLastWriteTime();

    bool didChange = time != mLastWriteTime;
    mLastWriteTime = time;

    return didChange;
  }

 private:
  std::filesystem::path                   mPath;
  mutable std::filesystem::file_time_type mLastWriteTime;
  mutable T                               mContent;
  mutable bool                            mIsLoaded;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_FILE_HPP
