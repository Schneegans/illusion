////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BindingState.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setBinding(BindingType const& value, uint32_t set, uint32_t binding) {
  if (getBinding(set, binding) != value) {
    mSetBindings[set][binding] = value;
    mDirtySetBindings.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setTexture(TexturePtr const& texture, uint32_t set, uint32_t binding) {
  setBinding(CombinedImageSamplerBinding{texture}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageImage(TexturePtr const& image, uint32_t set, uint32_t binding) {
  setBinding(StorageImageBinding{image, nullptr}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageImage(
  TexturePtr const& image, vk::ImageViewPtr const& view, uint32_t set, uint32_t binding) {
  setBinding(StorageImageBinding{image, view}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setUniformBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size,
  vk::DeviceSize offset, uint32_t set, uint32_t binding) {
  setBinding(UniformBufferBinding{buffer, size, offset}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setDynamicUniformBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size,
  uint32_t offset, uint32_t set, uint32_t binding) {
  setBinding(DynamicUniformBufferBinding{buffer, size}, set, binding);

  if (getDynamicOffset(set, binding) != offset) {
    mDynamicOffsets[set][binding] = offset;
    mDirtyDynamicOffsets.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size,
  vk::DeviceSize offset, uint32_t set, uint32_t binding) {
  setBinding(StorageBufferBinding{buffer, size, offset}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setDynamicStorageBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size,
  uint32_t offset, uint32_t set, uint32_t binding) {
  setBinding(DynamicStorageBufferBinding{buffer, size}, set, binding);

  if (getDynamicOffset(set, binding) != offset) {
    mDynamicOffsets[set][binding] = offset;
    mDirtyDynamicOffsets.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::reset(uint32_t set, uint32_t binding) {
  {
    auto setIt = mSetBindings.find(set);

    if (setIt != mSetBindings.end()) {
      auto bindingIt = setIt->second.find(binding);
      if (bindingIt != setIt->second.end()) {
        setIt->second.erase(bindingIt);
        mDirtySetBindings.insert(set);
      }
    }
  }

  {
    auto offsetIt = mDynamicOffsets.find(set);

    if (offsetIt != mDynamicOffsets.end()) {
      auto bindingIt = offsetIt->second.find(binding);
      if (bindingIt != offsetIt->second.end()) {
        offsetIt->second.erase(bindingIt);
        mDirtyDynamicOffsets.insert(set);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::reset(uint32_t set) {
  {
    auto setIt = mSetBindings.find(set);

    if (setIt != mSetBindings.end()) {
      mSetBindings.erase(setIt);
      mDirtySetBindings.insert(set);
    }
  }

  {
    auto offsetIt = mDynamicOffsets.find(set);

    if (offsetIt != mDynamicOffsets.end()) {
      mDynamicOffsets.erase(offsetIt);
      mDirtyDynamicOffsets.insert(set);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::reset() {
  for (auto const& setIt : mSetBindings) {
    mDirtySetBindings.insert(setIt.first);
  }
  mSetBindings.clear();

  for (auto const& offsetIt : mDynamicOffsets) {
    mDirtyDynamicOffsets.insert(offsetIt.first);
  }
  mDynamicOffsets.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<BindingType> BindingState::getBinding(uint32_t set, uint32_t binding) {
  auto setIt = mSetBindings.find(set);

  if (setIt == mSetBindings.end()) {
    return {};
  }

  auto bindingIt = setIt->second.find(binding);

  if (bindingIt == setIt->second.end()) {
    return {};
  }

  return bindingIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<uint32_t, BindingType> const& BindingState::getBindings(uint32_t set) {
  auto setIt = mSetBindings.find(set);

  if (setIt == mSetBindings.end()) {
    static std::map<uint32_t, BindingType> empty;
    return empty;
  }

  return setIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::set<uint32_t> const& BindingState::getDirtySets() const { return mDirtySetBindings; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::clearDirtySets() { mDirtySetBindings.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t BindingState::getDynamicOffset(uint32_t set, uint32_t binding) {
  auto setIt = mDynamicOffsets.find(set);

  if (setIt == mDynamicOffsets.end()) {
    return 0;
  }

  auto bindingIt = setIt->second.find(binding);

  if (bindingIt == setIt->second.end()) {
    return 0;
  }

  return bindingIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<uint32_t, uint32_t> const& BindingState::getDynamicOffsets(uint32_t set) {
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

void BindingState::clearDirtyDynamicOffsets() { mDirtyDynamicOffsets.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
