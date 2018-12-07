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

bool BindingState::StorageImageBinding::operator==(
  BindingState::StorageImageBinding const& other) const {
  return mImage == other.mImage;
}

bool BindingState::StorageImageBinding::operator!=(
  BindingState::StorageImageBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool BindingState::CombinedImageSamplerBinding::operator==(
  BindingState::CombinedImageSamplerBinding const& other) const {
  return mTexture == other.mTexture;
}

bool BindingState::CombinedImageSamplerBinding::operator!=(
  BindingState::CombinedImageSamplerBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool BindingState::DynamicUniformBufferBinding::operator==(
  BindingState::DynamicUniformBufferBinding const& other) const {
  return mBuffer == other.mBuffer && mSize == other.mSize;
}

bool BindingState::DynamicUniformBufferBinding::operator!=(
  BindingState::DynamicUniformBufferBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool BindingState::UniformBufferBinding::operator==(
  BindingState::UniformBufferBinding const& other) const {
  return mBuffer == other.mBuffer && mSize == other.mSize && mOffset == other.mOffset;
}

bool BindingState::UniformBufferBinding::operator!=(
  BindingState::UniformBufferBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setBinding(BindingType const& value, uint32_t set, uint32_t binding) {
  if (getBinding(set, binding) != value) {
    mBindings[set][binding] = value;
    mDirtySets.insert(set);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setTexture(
  std::shared_ptr<Texture> const& texture, uint32_t set, uint32_t binding) {

  setBinding(CombinedImageSamplerBinding{texture}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setStorageImage(
  std::shared_ptr<Texture> const& image, uint32_t set, uint32_t binding) {

  setBinding(StorageImageBinding{image}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setDynamicUniformBuffer(
  std::shared_ptr<BackedBuffer> const& buffer,
  vk::DeviceSize                       size,
  uint32_t                             set,
  uint32_t                             binding) {

  setBinding(DynamicUniformBufferBinding{buffer, size}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::setUniformBuffer(
  std::shared_ptr<BackedBuffer> const& buffer,
  vk::DeviceSize                       size,
  vk::DeviceSize                       offset,
  uint32_t                             set,
  uint32_t                             binding) {

  setBinding(UniformBufferBinding{buffer, size, offset}, set, binding);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<BindingState::BindingType> BindingState::getBinding(uint32_t set, uint32_t binding) {
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

Core::BitHash BindingState::getHash(uint32_t set) const {

  Core::BitHash hash;

  auto setIt = mBindings.find(set);
  if (setIt != mBindings.end()) {

    for (auto const& binding : setIt->second) {
      hash.push<16>(binding.first);

      if (std::holds_alternative<StorageImageBinding>(binding.second)) {
        hash.push<64>(std::get<StorageImageBinding>(binding.second).mImage.get());

      } else if (std::holds_alternative<CombinedImageSamplerBinding>(binding.second)) {
        hash.push<64>(std::get<CombinedImageSamplerBinding>(binding.second).mTexture.get());

      } else if (std::holds_alternative<DynamicUniformBufferBinding>(binding.second)) {
        hash.push<64>(std::get<DynamicUniformBufferBinding>(binding.second).mBuffer.get());
        hash.push<64>(std::get<DynamicUniformBufferBinding>(binding.second).mSize);

      } else if (std::holds_alternative<UniformBufferBinding>(binding.second)) {
        hash.push<64>(std::get<UniformBufferBinding>(binding.second).mBuffer.get());
        hash.push<64>(std::get<UniformBufferBinding>(binding.second).mSize);
        hash.push<64>(std::get<UniformBufferBinding>(binding.second).mOffset);
      }
    }
  }

  return hash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::set<uint32_t> const& BindingState::getDirtySets() const { return mDirtySets; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void BindingState::clearDirtySets() { mDirtySets.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
