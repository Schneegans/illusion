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

#include "../Core/BitHash.hpp"
#include "BindingTypes.hpp"

#include <map>
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This class is used by the CommandBuffer to track the descriptor set binding state.             //
// The BindingState stores what is bound to each descriptor set number. Whenever a binding        //
// changes, a dirty flag is set. This can be used to trigger descriptor set updates.              //
// In addition to to the bindings, dynamic offsets are be stored separately for each binding of   //
// each set. This means when a dynamic binding changes only by its offset (dynamic uniform buffer //
// or dynamic storage buffer), the corresponding set will not be flagged as dirty. Instead, the   //
// dynamic offset will be flagged as dirty which can be used to trigger a re-binding of the       //
// currently bound descriptor set.                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class BindingState {
 public:
  // Methods for setting the binding state ---------------------------------------------------------

  // BindingType is defined BindingTypes.hpp and is a std::variant containing one of the possible
  // binding targets (uniform buffer, storage image, ...).
  // This generic method is used by all the other set-methods below. You may use it, however the
  // explicit methods below are usually more convenient.
  void setBinding(BindingType const& value, uint32_t set, uint32_t binding);

  // Stores the given Texture as CombinedImageSamplerBinding.
  void setTexture(TexturePtr const& texture, uint32_t set, uint32_t binding);

  // Stores the given Texture as StorageImageBinding. It will use the ImageView of the texture,
  // which usually means that the entire base level of the texture will be bound for writing.
  void setStorageImage(TexturePtr const& image, uint32_t set, uint32_t binding);

  // Stores the given Texture as StorageImageBinding. The ImageView parameter can be used to store a
  // specific part of the Image (e.g. one specific mipmap level),
  void setStorageImage(
      TexturePtr const& image, vk::ImageViewPtr const& view, uint32_t set, uint32_t binding);

  // Stores the given BackedBuffer range as UniformBufferBinding.
  void setUniformBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size, vk::DeviceSize offset,
      uint32_t set, uint32_t binding);

  // Stores the given BackedBuffer range as DynamicUniformBufferBinding. When the same buffer and
  // size were bound before (only the offset changed) the set will not become dirty. Only the
  // dynamic offset for this set will be dirty which means that the currently bound descriptor sets
  // needs to be re-bound
  void setDynamicUniformBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size, uint32_t offset,
      uint32_t set, uint32_t binding);

  // Stores the given BackedBuffer range as StorageBufferBinding.
  void setStorageBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size, vk::DeviceSize offset,
      uint32_t set, uint32_t binding);

  // Stores the given BackedBuffer range as DynamicStorageBufferBinding. When the same buffer and
  // size were bound before (only the offset changed) the set will not become dirty. Only the
  // dynamic offset for this set will be dirty which means that the currently bound descriptor sets
  // needs to be re-bound
  void setDynamicStorageBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size, uint32_t offset,
      uint32_t set, uint32_t binding);

  // Removes the given binding for the given set. The dynamic offset (if set) will be removed as
  // well. The set and the dynamic offsets will be flagged as being dirty.
  void reset(uint32_t set, uint32_t binding);

  // Removes all bindings for the given set. The dynamic offsets (if there are any) will be removed
  // as well. The set and the dynamic offsets will be flagged as being dirty.
  void reset(uint32_t set);

  // Removes all bindings for all sets. The dynamic offsets (if there are any) will be removed as
  // well. All sets and dynamic offsets will be flagged as being dirty.
  void reset();

  // Methods for reading the binding state ---------------------------------------------------------

  // Retrieve all or specific bindings of given descriptor set numbers.
  std::optional<BindingType>             getBinding(uint32_t set, uint32_t binding);
  std::map<uint32_t, BindingType> const& getBindings(uint32_t set);

  // Get or clear the list of sets which have changed bindings.
  std::set<uint32_t> const& getDirtySets() const;
  void                      clearDirtySets();

  // Retrieve all or specific dynamic offsets of given descriptor set numbers. This will return
  // zero if the requested binding is actually not dynamic
  uint32_t                            getDynamicOffset(uint32_t set, uint32_t binding);
  std::map<uint32_t, uint32_t> const& getDynamicOffsets(uint32_t set);

  // Get or clear the list of sets which have changed dynamic offsets amongst their bindings.
  std::set<uint32_t> const& getDirtyDynamicOffsets() const;
  void                      clearDirtyDynamicOffsets();

 private:
  // Stores for each set number a map which stores for each binding number a BindingType
  std::map<uint32_t, std::map<uint32_t, BindingType>> mSetBindings;

  // Stores set numbers of descriptor sets with changed bindings
  std::set<uint32_t> mDirtySetBindings;

  // Stores for each set number a map which stores for each dynamic binding an offset
  std::map<uint32_t, std::map<uint32_t, uint32_t>> mDynamicOffsets;

  // Stores set numbers of descriptor sets with changed dynamic offsets
  std::set<uint32_t> mDirtyDynamicOffsets;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BINDING_STATE_HPP
