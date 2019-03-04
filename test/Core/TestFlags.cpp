////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Flags.hpp>

#include <doctest.h>

namespace Illusion::Core {

enum class Bits { eNone = 0, eBit1 = 1 << 0, eBit2 = 1 << 1, eBit3 = 1 << 2 };
typedef Core::Flags<Bits> Options;

Options operator|(Bits bit0, Bits bit1) {
  return Options(bit0) | bit1;
}

TEST_CASE("Illusion::Core::Flags") {
  Options options;

  // Default-constructed object should be initialized to zero.
  CHECK(options == Bits::eNone);

  // Cast to bool should return false.
  CHECK(!options);

  // Assigning a new value.
  options = Options(Bits::eBit1 | Bits::eBit2);
  CHECK(options.contains(Bits::eBit1));
  CHECK(options.contains(Bits::eBit2));
  CHECK(!options.contains(Bits::eBit3));

  // Removing one value.
  options ^= Bits::eBit2;
  CHECK(options.containsOnly(Bits::eBit1));

  // Masking with a non-contained bit should yield an empty set.
  options &= Bits::eBit3;
  CHECK(options == Bits::eNone);
}

} // namespace Illusion::Core
