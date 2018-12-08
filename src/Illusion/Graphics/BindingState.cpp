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
    mBindings[set][binding] = value;
    mDirtySets.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setTexture(TexturePtr const& texture, uint32_t set, uint32_t binding) {

  setBinding(CombinedImageSamplerBinding{texture}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageImage(TexturePtr const& image, uint32_t set, uint32_t binding) {

  setBinding(StorageImageBinding{image}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setDynamicUniformBuffer(
  BackedBufferPtr const& buffer, vk::DeviceSize size, uint32_t set, uint32_t binding) {

  setBinding(DynamicUniformBufferBinding{buffer, size}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setUniformBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size,
  vk::DeviceSize offset, uint32_t set, uint32_t binding) {

  setBinding(UniformBufferBinding{buffer, size, offset}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<BindingType> BindingState::getBinding(uint32_t set, uint32_t binding) {
  auto setIt = mBindings.find(set);

  if (setIt == mBindings.end()) {
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
  auto setIt = mBindings.find(set);

  if (setIt == mBindings.end()) {
    static std::map<uint32_t, BindingType> empty;
    return empty;
  }

  return setIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::set<uint32_t> const& BindingState::getDirtySets() const { return mDirtySets; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::clearDirtySets() { mDirtySets.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
