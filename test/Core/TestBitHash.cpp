////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/BitHash.hpp>

#include <doctest.h>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::BitHash") {
  BitHash hash;

  SUBCASE("Pushing bits") {
    hash.push<32>(42);
    CHECK(hash.size() == 32);
  }
}

} // namespace Illusion::Core
