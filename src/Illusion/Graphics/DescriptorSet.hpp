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

#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorSet : public vk::DescriptorSet {
 public:
  DescriptorSet(std::shared_ptr<Device> const& device, vk::DescriptorSet const& base);

  void bindCombinedImageSampler(std::shared_ptr<Texture> const& texture, uint32_t binding = 0);
  void bindStorageImage(std::shared_ptr<Texture> const& texture, uint32_t binding = 0);
  void bindUniformBuffer(
    std::shared_ptr<BackedBuffer> const& buffer,
    uint32_t                             binding = 0,
    vk::DeviceSize                       size    = 0,
    vk::DeviceSize                       offset  = 0);

 private:
  std::shared_ptr<Device> mDevice;
  uint32_t                mSet;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_SET_HPP
