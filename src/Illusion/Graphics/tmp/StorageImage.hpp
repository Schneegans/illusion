////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_STORAGE_IMAGE_HPP
#define ILLUSION_GRAPHICS_STORAGE_IMAGE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Context.hpp"
#include "Texture.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
template <typename T>
class StorageImage : public T {

 public:
  // -------------------------------------------------------------------------------- public members
  std::shared_ptr<Texture> mTexture;

  // -------------------------------------------------------------------------------- public methods
  StorageImage(std::shared_ptr<Context> const& context)
    : mContext(context) {}

  void bind(vk::DescriptorSet const& descriptorSet) const {
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eGeneral;
    imageInfo.imageView   = *mTexture->getImageView();
    imageInfo.sampler     = *mTexture->getSampler();

    vk::WriteDescriptorSet info;
    info.dstSet          = descriptorSet;
    info.dstBinding      = T::getBindingPoint();
    info.dstArrayElement = 0;
    info.descriptorType  = vk::DescriptorType::eStorageImage;
    info.descriptorCount = 1;
    info.pImageInfo      = &imageInfo;

    mContext->getDevice()->updateDescriptorSets(info, nullptr);
  }

 private:
  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Context> mContext;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_STORAGE_IMAGE_HPP
