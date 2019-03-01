////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Timer.hpp>

#include <doctest.h>
#include <thread>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::Timer") {

  // Create a auto-starting timer.
  Timer timer;

  // Wait for 0.1 second.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check the timer.
  CHECK(timer.getElapsed() == doctest::Approx(0.1).epsilon(0.001));

  // Reset and check again.
  timer.restart();
  CHECK(timer.getElapsed() == doctest::Approx(0.0).epsilon(0.001));

  // Wait for 0.1 second and check again.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  CHECK(timer.getElapsed() == doctest::Approx(0.1).epsilon(0.001));
}

} // namespace Illusion::Core
