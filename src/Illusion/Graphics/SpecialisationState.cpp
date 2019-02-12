////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SpecialisationState.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

void SpecialisationState::setIntegerConstant(uint32_t constantID, int32_t value) {
  set(constantID, *reinterpret_cast<uint32_t*>(&value));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SpecialisationState::setFloatConstant(uint32_t constantID, float value) {
  set(constantID, *reinterpret_cast<uint32_t*>(&value));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SpecialisationState::setBoolConstant(uint32_t constantID, bool value) {
  set(constantID, value ? 1u : 0u);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SpecialisationState::reset() {
  if (mValues.size() > 0) {
    mValues.clear();
    mDirty = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SpecializationInfo const* SpecialisationState::getInfo() const {
  update();

  if (mValues.size() == 0) {
    return nullptr;
  }

  return &mInfo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Core::BitHash SpecialisationState::getHash() const {
  update();

  return mHash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SpecialisationState::set(uint32_t constantID, uint32_t value) {
  auto it = mValues.find(constantID);
  if (it != mValues.end() && it->second == value) {
    return;
  }

  mValues[constantID] = value;
  mDirty              = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SpecialisationState::update() const {
  if (mDirty) {

    mHash.clear();
    mData.clear();
    mDataEntries.clear();

    for (auto const& value : mValues) {
      mDataEntries.emplace_back(
          value.first, mDataEntries.size() * sizeof(uint32_t), sizeof(uint32_t));
      mData.emplace_back(value.second);
    }

    mInfo.mapEntryCount = mDataEntries.size();
    mInfo.pMapEntries   = mDataEntries.data();
    mInfo.dataSize      = mData.size() * sizeof(uint32_t);
    mInfo.pData         = mData.data();

    // Update hash.
    for (auto const& value : mValues) {
      mHash.push<32>(value.first);
      mHash.push<32>(value.second);
    }

    mDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
