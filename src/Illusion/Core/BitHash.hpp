////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_BITHASH_HPP
#define ILLUSION_CORE_BITHASH_HPP

// ---------------------------------------------------------------------------------------- includes
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

    for (int i(0); i < bitCount; ++i) {
      push_back((castedValue & mask));
      mask = mask << 1;
    }
  }
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_BITHASH_HPP
