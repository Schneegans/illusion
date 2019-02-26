////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/File.hpp>

#include <doctest.h>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::File") {
  File testFile("invalid.txt");

  // At first, this file should not exist.
  CHECK(testFile.isValid() == false);

  // We should have a valid file name nevertheless.
  CHECK(testFile.getFileName() == "invalid.txt");

  // Change the file name.
  testFile.setFileName("testFile.txt");
  CHECK(testFile.getFileName() == "testFile.txt");

  // We should have a last write time of 0 for invalid files.
  CHECK(testFile.getLastWriteTime() == 0);

  // Now we write something.
  std::string write = "Foo Bar";
  testFile.save(write);

  // Then the file should exist.
  CHECK(testFile.isValid() == true);

  // And we should be able to read it again.
  auto read = testFile.getContent<std::string>();
  CHECK(write == read);

  // And the last write time should be != 0.
  CHECK(testFile.getLastWriteTime() != 0);

  // However, changedOnDisc should return false as we were writing it for the last time.
  CHECK(!testFile.changedOnDisc());

  // Then we remove the file again.
  testFile.remove();

  // Which should make it invalid again.
  CHECK(testFile.isValid() == false);
}

} // namespace Illusion::Core
