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
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class BitHash : public std::vector<bool> {

 public:
  // -------------------------------------------------------------------------------- public methods
  template <uint32_t bits, typename T>
  void push(T const& value) {

    static_assert(bits <= 64, "Cannot push more than 64 bits into BitHash!");
    static_assert(bits <= sizeof(T) * 8, "Cannot push more bits into the BitHash than T has!");

    uint64_t mask(1);
    uint64_t castedValue(0);

    std::memcpy(&castedValue, &value, sizeof(T));

    for (int i(0); i < bits; ++i) {
      push_back((castedValue & mask));
      mask = mask << 1;
    }
  }
};

// -------------------------------------------------------------------------------------------------
} // namespace Illusion::Core

#endif // ILLUSION_CORE_BITHASH_HPP
