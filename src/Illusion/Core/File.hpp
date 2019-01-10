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

#include <fstream>
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This class is used to read and write text files. The template parameter specifies the type the //
// content of the file is returned as. Common use cases include:                                  //
//                                                                                                //
// File file;                                                                                     //
// auto content = file.getContent<std::string>(); // reads the content as a std::string           //
// auto content = file.getContent<std::vector<uint8_t>>(); // reads the content as a byte array   //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Illusion::Core {

class File {

 public:
  // This constructs a invalid File.
  File() = default;

  // This constructs a File for a given name.
  File(std::string const& fileName);

  // Returns if the given file is valid.
  bool isValid() const;

  // Returns the given file's content. The template parameter should be either std::string or some
  // other container like a std::vector.
  template <typename T>
  T getContent() const {

    std::ifstream ifs(mPath, std::ifstream::in | std::ios::binary);
    if (!ifs) {
      ILLUSION_WARNING << "Cannot open file \"" << mPath << "\"!" << std::endl;
      return T();
    }

    ifs.seekg(0, std::ios::end);
    size_t filesize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    T data;
    data.resize(filesize / sizeof(typename T::value_type));

    ifs.read((char*)data.data(), filesize);
    ifs.close();

    mLastWriteTime = getLastWriteTime();

    return data;
  }

  // Saves the file. The template parameter should be either std::string or some other container
  // like a std::vector.
  template <typename T>
  bool save(T const& data) const {
    std::ofstream ofs(mPath, std::ifstream::out | std::ios::binary);
    if (!ofs) {
      ILLUSION_WARNING << "Cannot open file \"" << mPath << "\"!" << std::endl;
      return false;
    }

    ofs.write((char*)data.data(), data.size() * sizeof(typename T::value_type));
    ofs.close();

    return true;
  }

  // Deletes the file from the file system
  void remove();

  // Returns the file's name.
  std::string const& getFileName() const;

  // Sets the file name. This is mostly for defaul-constructed files. This method does not rename
  // the file on disc, it rather makes this instance point to another file.
  void setFileName(std::string const& path);

  // Returns the last write time as reported from std::filesystem
  time_t getLastWriteTime() const;

  // Polls for changes to this file
  bool changedOnDisc() const;
  void resetChangedOnDisc();

 private:
  std::string    mPath;
  mutable time_t mLastWriteTime;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_FILE_HPP
