////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_BINDING_TYPES_HPP
#define ILLUSION_GRAPHICS_BINDING_TYPES_HPP

#include "fwd.hpp"

#include <variant>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

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

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BINDING_TYPES_HPP
