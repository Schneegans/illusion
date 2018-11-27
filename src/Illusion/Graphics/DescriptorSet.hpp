////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DESCRIPTOR_SET_HPP
#define ILLUSION_GRAPHICS_DESCRIPTOR_SET_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class DescriptorSet : public vk::DescriptorSet {
 public:
  DescriptorSet(std::shared_ptr<Device> const& device, uint32_t set, vk::DescriptorSet const& base);

  void bindCombinedImageSampler(std::shared_ptr<Texture> const& texture, uint32_t binding);
  void bindStorageImage(std::shared_ptr<Texture> const& texture, uint32_t binding);
  void bindUniformBuffer(std::shared_ptr<BackedBuffer> const& buffer, uint32_t binding);

  uint32_t getSet() const;

 private:
  std::shared_ptr<Device> mDevice;
  uint32_t                mSet;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_SET_HPP
