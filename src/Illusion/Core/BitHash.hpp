////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_BITHASH_HPP
#define ILLUSION_CORE_BITHASH_HPP

#include <cstdint>
#include <cstring>
#include <vector>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A small class which can be used to create hashes for complex objects. This is done by pushing  //
// individual bits of the members of the object into an std::vector<bool>. For example consider a //
// struct like this:                                                                              //
//                                                                                                //
// struct Vehicle {                                                                               //
//   enum class Type { eBike = 0, eCar = 1, eBoot = 2, eAirplane = 3 };                           //
//   uint32_t mPrice;                                                                             //
//   Type     mType;                                                                              //
// };                                                                                             //
//                                                                                                //
// You could then create a hash like this:                                                        //
//                                                                                                //
// BitHash hash;                                                                                  //
// hash.push<32>(vehicle.mPrice); // mPrice is a uint32_t, therefore we have to push 32 bits      //
// hash.push<2>(vehicle.mType);   // mType only needs 2 bits                                      //
//                                                                                                //
// Then we can store our Vehicles in a maplike this:                                              //
//                                                                                                //
// std::map<BitHash, Vehicle> mCache;                                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class BitHash : public std::vector<bool> {

 public:
  template <uint32_t bitCount, typename T>
  void push(T const& value) {

    static_assert(bitCount <= 64, "Cannot push more than 64 bits into BitHash!");
    static_assert(bitCount <= sizeof(T) * 8, "Cannot push more bits into the BitHash than T has!");

    uint64_t mask(1);
    uint64_t castedValue(0);

    std::memcpy(&castedValue, &value, sizeof(T));

    for (uint32_t i(0); i < bitCount; ++i) {
      push_back((castedValue & mask));
      mask = mask << 1;
    }
  }
};

// tests -------------------------------------------------------------------------------------------
#ifdef DOCTEST_LIBRARY_INCLUDED

TEST_CASE("Illusion::Core::BitHash") {
  BitHash hash;

  SUBCASE("Pushing bits") {
    hash.push<32>(42);
    CHECK(hash.size() == 32);
  }
}

#endif

} // namespace Illusion::Core

#endif // ILLUSION_CORE_BITHASH_HPP
