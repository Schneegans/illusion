////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Signal.hpp>

#include <doctest.h>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::Signal") {

  Signal<> simpleSignal;

  SUBCASE("Testing Signal connections") {
    bool result = false;

    // When the signal is emitted, our result value is set to true.
    simpleSignal.connect([&]() {
      result = true;
      return true;
    });

    simpleSignal.emit();

    CHECK(result);
  }

  SUBCASE("Testing multiple Signal connections") {
    uint32_t count = 0;

    // When the signal is emitted, our count value is increased by one. We connect 100 copies of
    // this lambda; so our count value should be 100 in the end.
    for (uint32_t i(0); i < 100; ++i) {
      simpleSignal.connect([&]() {
        ++count;
        return true;
      });
    }

    simpleSignal.emit();

    CHECK(count == 100);
  }

  SUBCASE("Testing Signals disconnectAll") {
    uint32_t count = 0;

    // When the signal is emitted, our count value is increased by one.
    simpleSignal.connect([&]() {
      ++count;
      return true;
    });

    // We emit it once.
    simpleSignal.emit();

    // Then disconnect our lambda.
    simpleSignal.disconnectAll();

    // And emit it once more.
    simpleSignal.emit();

    CHECK(count == 1);
  }

  SUBCASE("Testing Signal auto disconnect") {
    uint32_t count = 0;

    // When the signal is emitted, our count value is increased by one. We return false from within
    // the lambda, this should make the lambda be disconnected.
    simpleSignal.connect([&]() {
      ++count;
      return false;
    });

    // We emit it twice.
    simpleSignal.emit();
    simpleSignal.emit();

    CHECK(count == 1);
  }
}

} // namespace Illusion::Core
