////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_FLAGS_HPP
#define ILLUSION_CORE_FLAGS_HPP

#include <type_traits>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Based on the implementation in vulkan.hpp                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename FlagBitsType>
struct FlagTraits {
  enum { allFlags = 0 };
};

template <typename BitType>
class Flags {
 public:
  typedef typename std::underlying_type<BitType>::type MaskType;

  Flags()
      : mMask(0) {
  }

  Flags(BitType bit)
      : mMask(static_cast<MaskType>(bit)) {
  }

  Flags(Flags<BitType> const& rhs)
      : mMask(rhs.mMask) {
  }

  explicit Flags(MaskType flags)
      : mMask(flags) {
  }

  Flags<BitType>& operator=(Flags<BitType> const& rhs) {
    mMask = rhs.mMask;
    return *this;
  }

  Flags<BitType>& operator|=(Flags<BitType> const& rhs) {
    mMask |= rhs.mMask;
    return *this;
  }

  Flags<BitType>& operator&=(Flags<BitType> const& rhs) {
    mMask &= rhs.mMask;
    return *this;
  }

  Flags<BitType>& operator^=(Flags<BitType> const& rhs) {
    mMask ^= rhs.mMask;
    return *this;
  }

  Flags<BitType> operator|(Flags<BitType> const& rhs) const {
    Flags<BitType> result(*this);
    result |= rhs;
    return result;
  }

  Flags<BitType> operator&(Flags<BitType> const& rhs) const {
    Flags<BitType> result(*this);
    result &= rhs;
    return result;
  }

  Flags<BitType> operator^(Flags<BitType> const& rhs) const {
    Flags<BitType> result(*this);
    result ^= rhs;
    return result;
  }

  bool operator!() const {
    return !mMask;
  }

  Flags<BitType> operator~() const {
    Flags<BitType> result(*this);
    result.mMask ^= FlagTraits<BitType>::allFlags;
    return result;
  }

  bool operator==(Flags<BitType> const& rhs) const {
    return mMask == rhs.mMask;
  }

  bool operator!=(Flags<BitType> const& rhs) const {
    return mMask != rhs.mMask;
  }

  explicit operator bool() const {
    return !!mMask;
  }

  explicit operator MaskType() const {
    return mMask;
  }

 private:
  MaskType mMask;
};

template <typename BitType>
Flags<BitType> operator|(BitType bit, Flags<BitType> const& flags) {
  return flags | bit;
}

template <typename BitType>
Flags<BitType> operator&(BitType bit, Flags<BitType> const& flags) {
  return flags & bit;
}

template <typename BitType>
Flags<BitType> operator^(BitType bit, Flags<BitType> const& flags) {
  return flags ^ bit;
}

} // namespace Illusion::Core

#endif // ILLUSION_CORE_FLAGS_HPP
