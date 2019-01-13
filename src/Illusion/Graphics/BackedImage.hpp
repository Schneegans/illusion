////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
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
