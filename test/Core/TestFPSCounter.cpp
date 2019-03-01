////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/FPSCounter.hpp>

#include <doctest.h>
#include <thread>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::FPSCounter") {

  // Create a auto-starting FPSCounter.
  FPSCounter<20> counter;

  for (size_t i(0); i < 50; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    counter.step();
    CHECK(counter.pFrameTime.get() == doctest::Approx(0.01).epsilon(0.001));
    CHECK(counter.pFPS.get() == doctest::Approx(100).epsilon(0.1));
  }
}

} // namespace Illusion::Core
