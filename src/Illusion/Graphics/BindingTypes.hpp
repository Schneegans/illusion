////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_BINDING_TYPES_HPP
#define ILLUSION_GRAPHICS_BINDING_TYPES_HPP

#include "fwd.hpp"

#include <variant>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// These structs are used by the BindingState. Especially the std::variant at the bottom of this  //
// file is used extensively to track the current descriptor set binding state (aliased as         //
// BindingType). See BindingState.hpp for details.                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct InputAttachmentBinding {
  BackedImageConstPtr mAttachment;

  bool operator==(InputAttachmentBinding const& other) const;
  bool operator!=(InputAttachmentBinding const& other) const;
};

// -------------------------------------------------------------------------------------------------

struct StorageImageBinding {
  TextureConstPtr  mImage;
  vk::ImageViewPtr mView;

  bool operator==(StorageImageBinding const& other) const;
  bool operator!=(StorageImageBinding const& other) const;
};

// -------------------------------------------------------------------------------------------------

struct CombinedImageSamplerBinding {
  TextureConstPtr mTexture;

  bool operator==(CombinedImageSamplerBinding const& other) const;
  bool operator!=(CombinedImageSamplerBinding const& other) const;
};

// -------------------------------------------------------------------------------------------------

struct UniformBufferBinding {
  BackedBufferConstPtr mBuffer;
  vk::DeviceSize       mSize;
  vk::DeviceSize       mOffset;

  bool operator==(UniformBufferBinding const& other) const;
  bool operator!=(UniformBufferBinding const& other) const;
};

// -------------------------------------------------------------------------------------------------

struct DynamicUniformBufferBinding {
  BackedBufferConstPtr mBuffer;
  vk::DeviceSize       mSize;

  bool operator==(DynamicUniformBufferBinding const& other) const;
  bool operator!=(DynamicUniformBufferBinding const& other) const;
};

// -------------------------------------------------------------------------------------------------

struct StorageBufferBinding {
  BackedBufferConstPtr mBuffer;
  vk::DeviceSize       mSize;
  vk::DeviceSize       mOffset;

  bool operator==(StorageBufferBinding const& other) const;
  bool operator!=(StorageBufferBinding const& other) const;
};

// -------------------------------------------------------------------------------------------------

struct DynamicStorageBufferBinding {
  BackedBufferConstPtr mBuffer;
  vk::DeviceSize       mSize;

  bool operator==(DynamicStorageBufferBinding const& other) const;
  bool operator!=(DynamicStorageBufferBinding const& other) const;
};

// -------------------------------------------------------------------------------------------------

typedef std::variant<InputAttachmentBinding, StorageImageBinding, CombinedImageSamplerBinding,
    UniformBufferBinding, DynamicUniformBufferBinding, StorageBufferBinding,
    DynamicStorageBufferBinding>
    BindingType;

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BINDING_TYPES_HPP
