////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_BINDING_STATE_HPP
#define ILLUSION_GRAPHICS_BINDING_STATE_HPP

#include "fwd.hpp"

#include "../Core/BitHash.hpp"

#include <map>
#include <set>
#include <variant>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class BindingState {
 public:
  struct StorageImageBinding {
    std::shared_ptr<Texture> mImage;

    bool operator==(StorageImageBinding const& other) const;
    bool operator!=(StorageImageBinding const& other) const;
  };

  struct CombinedImageSamplerBinding {
    std::shared_ptr<Texture> mTexture;

    bool operator==(CombinedImageSamplerBinding const& other) const;
    bool operator!=(CombinedImageSamplerBinding const& other) const;
  };

  struct DynamicUniformBufferBinding {
    std::shared_ptr<BackedBuffer> mBuffer;
    vk::DeviceSize                mSize;

    bool operator==(DynamicUniformBufferBinding const& other) const;
    bool operator!=(DynamicUniformBufferBinding const& other) const;
  };

  struct UniformBufferBinding {
    std::shared_ptr<BackedBuffer> mBuffer;
    vk::DeviceSize                mSize;
    vk::DeviceSize                mOffset;

    bool operator==(UniformBufferBinding const& other) const;
    bool operator!=(UniformBufferBinding const& other) const;
  };

  typedef std::variant<
    StorageImageBinding,
    CombinedImageSamplerBinding,
    DynamicUniformBufferBinding,
    UniformBufferBinding>
    BindingType;

  void setBinding(BindingType const& value, uint32_t set, uint32_t binding);

  void setTexture(std::shared_ptr<Texture> const& texture, uint32_t set, uint32_t binding);
  void setStorageImage(std::shared_ptr<Texture> const& image, uint32_t set, uint32_t binding);
  void setDynamicUniformBuffer(
    std::shared_ptr<BackedBuffer> const& buffer,
    vk::DeviceSize                       size,
    uint32_t                             set,
    uint32_t                             binding);
  void setUniformBuffer(
    std::shared_ptr<BackedBuffer> const& buffer,
    vk::DeviceSize                       size,
    vk::DeviceSize                       offset,
    uint32_t                             set,
    uint32_t                             binding);

  std::optional<BindingType> getBinding(uint32_t set, uint32_t binding);

  Core::BitHash             getHash(uint32_t set) const;
  std::set<uint32_t> const& getDirtySets() const;
  void                      clearDirtySets();

 private:
  std::map<uint32_t, std::map<uint32_t, BindingType>> mBindings;

  // Dirty State -----------------------------------------------------------------------------------
  std::set<uint32_t> mDirtySets;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BINDING_STATE_HPP
