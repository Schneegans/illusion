////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BindingTypes.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

bool StorageImageBinding::operator==(StorageImageBinding const& other) const {
  return mImage == other.mImage && mView == other.mView;
}

bool StorageImageBinding::operator!=(StorageImageBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool CombinedImageSamplerBinding::operator==(CombinedImageSamplerBinding const& other) const {
  return mTexture == other.mTexture;
}

bool CombinedImageSamplerBinding::operator!=(CombinedImageSamplerBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool UniformBufferBinding::operator==(UniformBufferBinding const& other) const {
  return mBuffer == other.mBuffer && mSize == other.mSize && mOffset == other.mOffset;
}

bool UniformBufferBinding::operator!=(UniformBufferBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool DynamicUniformBufferBinding::operator==(DynamicUniformBufferBinding const& other) const {
  return mBuffer == other.mBuffer && mSize == other.mSize;
}

bool DynamicUniformBufferBinding::operator!=(DynamicUniformBufferBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool StorageBufferBinding::operator==(StorageBufferBinding const& other) const {
  return mBuffer == other.mBuffer && mSize == other.mSize && mOffset == other.mOffset;
}

bool StorageBufferBinding::operator!=(StorageBufferBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool DynamicStorageBufferBinding::operator==(DynamicStorageBufferBinding const& other) const {
  return mBuffer == other.mBuffer && mSize == other.mSize;
}

bool DynamicStorageBufferBinding::operator!=(DynamicStorageBufferBinding const& other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
