////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Color.hpp>

#include <doctest.h>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::Color") {
  Color color("rgba(255, 127, 0, 0.2)");

  SUBCASE("Checking from-string constructor") {
    CHECK(color.r() == 1.f);
    CHECK(color.g() == 127.f / 255.f);
    CHECK(color.b() == 0.f);
    CHECK(color.a() == 0.2f);
  }
}

} // namespace Illusion::Core
