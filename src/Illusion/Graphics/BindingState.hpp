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
////////////////////////////////////////////////////////////////////////////////////////////////////

class BindingState {
 public:
  void setBinding(BindingType const& value, uint32_t set, uint32_t binding);

  void setTexture(TexturePtr const& texture, uint32_t set, uint32_t binding);

  void setStorageImage(TexturePtr const& image, uint32_t set, uint32_t binding);
  void setStorageImage(
    TexturePtr const& image, vk::ImageViewPtr const& view, uint32_t set, uint32_t binding);

  void setDynamicUniformBuffer(
    BackedBufferPtr const& buffer, vk::DeviceSize size, uint32_t set, uint32_t binding);

  void setUniformBuffer(BackedBufferPtr const& buffer, vk::DeviceSize size, vk::DeviceSize offset,
    uint32_t set, uint32_t binding);

  std::optional<BindingType>             getBinding(uint32_t set, uint32_t binding);
  std::map<uint32_t, BindingType> const& getBindings(uint32_t set);

  void clearSet(uint32_t set);

  std::set<uint32_t> const& getDirtySets() const;
  void                      clearDirtySets();

 private:
  std::map<uint32_t, std::map<uint32_t, BindingType>> mBindings;
  std::set<uint32_t>                                  mDirtySets;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BINDING_STATE_HPP
