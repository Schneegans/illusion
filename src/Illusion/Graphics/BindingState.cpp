////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BindingState.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setBinding(BindingType const& value, uint32_t set, uint32_t binding) {
  // Check whether the thing is already bound to this binding location.
  if (getBinding(set, binding) != value) {
    mSetBindings[set][binding] = value;
    mDirtySetBindings.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setInputAttachment(
    BackedImageConstPtr const& attachment, uint32_t set, uint32_t binding) {
  setBinding(InputAttachmentBinding{attachment}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setTexture(TextureConstPtr const& texture, uint32_t set, uint32_t binding) {
  setBinding(CombinedImageSamplerBinding{texture}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageImage(TextureConstPtr const& image, uint32_t set, uint32_t binding) {
  setBinding(StorageImageBinding{image, nullptr}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageImage(
    TextureConstPtr const& image, vk::ImageViewPtr const& view, uint32_t set, uint32_t binding) {
  setBinding(StorageImageBinding{image, view}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setUniformBuffer(BackedBufferConstPtr const& buffer, vk::DeviceSize size,
    vk::DeviceSize offset, uint32_t set, uint32_t binding) {
  setBinding(UniformBufferBinding{buffer, size, offset}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setDynamicUniformBuffer(BackedBufferConstPtr const& buffer, vk::DeviceSize size,
    uint32_t offset, uint32_t set, uint32_t binding) {
  setBinding(DynamicUniformBufferBinding{buffer, size}, set, binding);

  // Check whether the dynamic offset changed. If so, set it to dirty.
  if (getDynamicOffset(set, binding) != offset) {
    mDynamicOffsets[set][binding] = offset;
    mDirtyDynamicOffsets.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageBuffer(BackedBufferConstPtr const& buffer, vk::DeviceSize size,
    vk::DeviceSize offset, uint32_t set, uint32_t binding) {
  setBinding(StorageBufferBinding{buffer, size, offset}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setDynamicStorageBuffer(BackedBufferConstPtr const& buffer, vk::DeviceSize size,
    uint32_t offset, uint32_t set, uint32_t binding) {
  setBinding(DynamicStorageBufferBinding{buffer, size}, set, binding);

  // Check whether the dynamic offset changed. If so, set it to dirty.
  if (getDynamicOffset(set, binding) != offset) {
    mDynamicOffsets[set][binding] = offset;
    mDirtyDynamicOffsets.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::reset(uint32_t set, uint32_t binding) {

  // Find the thing bound to the given binding location, erase it and set it to dirty.
  auto setIt = mSetBindings.find(set);
  if (setIt != mSetBindings.end()) {
    auto bindingIt = setIt->second.find(binding);
    if (bindingIt != setIt->second.end()) {
      setIt->second.erase(bindingIt);
      mDirtySetBindings.insert(set);
    }
  }

  // Find any dynamic offset for that particulat binding location, erase it and set it to dirty.
  auto offsetIt = mDynamicOffsets.find(set);
  if (offsetIt != mDynamicOffsets.end()) {
    auto bindingIt = offsetIt->second.find(binding);
    if (bindingIt != offsetIt->second.end()) {
      offsetIt->second.erase(bindingIt);
      mDirtyDynamicOffsets.insert(set);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::reset(uint32_t set) {

  // Check whether there is anything to reset and if so, set the set to be dirty.
  auto setIt = mSetBindings.find(set);
  if (setIt != mSetBindings.end()) {
    mSetBindings.erase(setIt);
    mDirtySetBindings.insert(set);
  }

  // Check whether there is any dynamic offset in this set and if so, set the offset to be dirty.
  auto offsetIt = mDynamicOffsets.find(set);
  if (offsetIt != mDynamicOffsets.end()) {
    mDynamicOffsets.erase(offsetIt);
    mDirtyDynamicOffsets.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::reset() {

  // First set all sets we have to be dirty, then clear all bindings.
  for (auto const& setIt : mSetBindings) {
    mDirtySetBindings.insert(setIt.first);
  }
  mSetBindings.clear();

  // Do the same for all dynamic offsets.
  for (auto const& offsetIt : mDynamicOffsets) {
    mDirtyDynamicOffsets.insert(offsetIt.first);
  }
  mDynamicOffsets.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<BindingType> BindingState::getBinding(uint32_t set, uint32_t binding) {

  // First find the requested set.
  auto setIt = mSetBindings.find(set);
  if (setIt == mSetBindings.end()) {
    return {};
  }

  // Then find the binding in this set.
  auto bindingIt = setIt->second.find(binding);
  if (bindingIt == setIt->second.end()) {
    return {};
  }

  return bindingIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<uint32_t, BindingType> const& BindingState::getBindings(uint32_t set) {

  // Find all bindings of the requested set. If there is none, return a reference to an empty static
  // set of bindings.
  auto setIt = mSetBindings.find(set);
  if (setIt == mSetBindings.end()) {
    static std::map<uint32_t, BindingType> empty;
    return empty;
  }

  return setIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::set<uint32_t> const& BindingState::getDirtySets() const {
  return mDirtySetBindings;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::clearDirtySets() {
  mDirtySetBindings.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t BindingState::getDynamicOffset(uint32_t set, uint32_t binding) {

  // First find the requested set.
  auto setIt = mDynamicOffsets.find(set);
  if (setIt == mDynamicOffsets.end()) {
    return 0;
  }

  // Then find the dynamic offsets in this set.
  auto bindingIt = setIt->second.find(binding);
  if (bindingIt == setIt->second.end()) {
    return 0;
  }

  return bindingIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<uint32_t, uint32_t> const& BindingState::getDynamicOffsets(uint32_t set) {

  // Find all dynamic offsets of the requested set. If there is none, return a reference to an empty
  // static set of dynamic offsets.
  auto setIt = mDynamicOffsets.find(set);
  if (setIt == mDynamicOffsets.end()) {
    static std::map<uint32_t, uint32_t> empty;
    return empty;
  }

  return setIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::set<uint32_t> const& BindingState::getDirtyDynamicOffsets() const {
  return mDirtyDynamicOffsets;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::clearDirtyDynamicOffsets() {
  mDirtyDynamicOffsets.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
