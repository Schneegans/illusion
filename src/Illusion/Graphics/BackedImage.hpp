////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_BACKED_IMAGE_HPP
#define ILLUSION_GRAPHICS_BACKED_IMAGE_HPP

#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A BackedImage stores a vk::Image, a vk::ImageView for this image and the vk::DeviceMemory      //
// backing the image. Additionally all create-info objects are stored in order to access the      //
// properties of the  image, the view and the memory. Use the Device class to easily create a     //
// BackedImage.                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackedImage {
  vk::ImagePtr        mImage;
  vk::ImageViewPtr    mView;
  vk::DeviceMemoryPtr mMemory;

  vk::ImageCreateInfo     mImageInfo;
  vk::ImageViewCreateInfo mViewInfo;
  vk::MemoryAllocateInfo  mMemoryInfo;

  vk::ImageLayout mCurrentLayout = vk::ImageLayout::eUndefined;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BACKED_IMAGE_HPP
