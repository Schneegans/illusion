////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SPECIALISATION_STATE_HPP
#define ILLUSION_GRAPHICS_SPECIALISATION_STATE_HPP

#include "fwd.hpp"

#include "../Core/BitHash.hpp"

#include <map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SpecialisationState {
 public:
  SpecialisationState() = default;

  void setIntegerConstant(uint32_t constantID, int32_t value);
  void setFloatConstant(uint32_t constantID, float value);
  void setBoolConstant(uint32_t constantID, bool value);

  void reset();

  vk::SpecializationInfo const* getInfo() const;
  Core::BitHash                 getHash() const;

 private:
  void set(uint32_t constantID, uint32_t value);
  void update() const;

  std::map<uint32_t, uint32_t> mValues;

  // Lazy State ------------------------------------------------------------------------------------
  mutable vk::SpecializationInfo                  mInfo;
  mutable std::vector<vk::SpecializationMapEntry> mDataEntries;
  mutable std::vector<uint32_t>                   mData;

  // Dirty State -----------------------------------------------------------------------------------
  mutable bool          mDirty = true;
  mutable Core::BitHash mHash;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SPECIALISATION_STATE_HPP
