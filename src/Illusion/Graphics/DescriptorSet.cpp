////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "DescriptorSet.hpp"

#include "../Core/Logger.hpp"
#include "Context.hpp"
#include "Texture.hpp"

#include <iostream>

namespace Illusion::Graphics {

DescriptorSet::DescriptorSet(
  std::shared_ptr<Context> const& context, vk::DescriptorSet const& base, uint32_t set)
  : vk::DescriptorSet(base)
  , mContext(context)
  , mSet(set) {}

void DescriptorSet::bindCombinedImageSampler(
  std::shared_ptr<Texture> const& texture, uint32_t binding) {
  vk::DescriptorImageInfo imageInfo;
  imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  imageInfo.imageView   = *texture->getImageView();
  imageInfo.sampler     = *texture->getSampler();

  vk::WriteDescriptorSet info;
  info.dstSet          = *this;
  info.dstBinding      = binding;
  info.dstArrayElement = 0;
  info.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
  info.descriptorCount = 1;
  info.pImageInfo      = &imageInfo;

  mContext->getDevice()->updateDescriptorSets(info, nullptr);
}

} // namespace Illusion::Graphics
