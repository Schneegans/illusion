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
// This class is used by the CommandBuffer to track the values of specialization constants. For   //
// now, it only supports specialization of int32_t, float and bool scalars. In Illusion           //
// specialization constants are part of the pipeline state of a CommandBuffer and will trigger    //
// pipeline recreations when modified.                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////

class SpecialisationState {
 public:
  // Sets a scalar integer specialization constant to the given value. You have to ensure that the
  // specialization constant with this ID is actually of that type. Else undefined behavior awaits.
  void setIntegerConstant(uint32_t constantID, int32_t value);

  // Sets a scalar float specialization constant to the given value. You have to ensure that the
  // specialization constant with this ID is actually of that type. Else undefined behavior awaits.
  void setFloatConstant(uint32_t constantID, float value);

  // Sets a scalar bool specialization constant to the given value. You have to ensure that the
  // specialization constant with this ID is actually of that type. Else undefined behavior awaits.
  void setBoolConstant(uint32_t constantID, bool value);

  // Deletes the stored value for the given constantID. If the current shader has a specialization
  // constant with this ID, it will use the default value afterwards.
  void reset(uint32_t constantID);

  // Deletes all stored values. The current shader will use the default value afterwards.
  void reset();

  // Returns a vk::SpecializationInfo based on the values set above. This pointer stays valid until
  // any value is set with the methods above. Therefore it should only used temporarily.
  vk::SpecializationInfo const* getInfo() const;

  // Returns a hash to uniquely identify a set of specialization constants. This is used by the
  // CommandBuffer to determine whether a new vk::Pipeline has to be created.
  Core::BitHash getHash() const;

 private:
  void set(uint32_t constantID, uint32_t value);
  void update() const;

  // Maps constant id to a value. The value is the reinterpreted int32_t, float or bool.
  std::map<uint32_t, uint32_t> mValues;

  // Dirty State -----------------------------------------------------------------------------------
  mutable bool                                    mDirty = true;
  mutable vk::SpecializationInfo                  mInfo;
  mutable std::vector<vk::SpecializationMapEntry> mDataEntries;
  mutable std::vector<uint32_t>                   mData;
  mutable Core::BitHash                           mHash;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SPECIALISATION_STATE_HPP
