////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/ScopedTimer.hpp>

#include <doctest.h>
#include <iostream>
#include <sstream>
#include <thread>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::ScopedTimer") {
  // Redirect cout to a ostringstream.
  std::ostringstream oss;
  auto               coutBuffer = std::cout.rdbuf();
  std::cout.rdbuf(oss.rdbuf());

  {
    ScopedTimer timer("Foo");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Restore normal cout behavior.
  std::cout.rdbuf(coutBuffer);

  // Check that there was some output. Pretty difficult to chack for the correct output, though.
  CHECK(oss);
  CHECK(oss.str() != "");
}

} // namespace Illusion::Core
