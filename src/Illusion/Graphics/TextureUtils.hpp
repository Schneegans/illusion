////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_TEXTURE_UTILS_HPP
#define ILLUSION_GRAPHICS_TEXTURE_UTILS_HPP

#include "fwd.hpp"

namespace Illusion::Graphics::TextureUtils {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool formatSupportsLinearSampling(DevicePtr const& device, vk::Format format);

uint32_t getMaxMipmapLevels(uint32_t width, uint32_t height);

TexturePtr createFromFile(DevicePtr const& device, std::string const& fileName,
  vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo(), bool generateMipmaps = true);

TexturePtr createCubemapFrom360PanoramaFile(DevicePtr const& device, std::string const& fileName,
  int32_t size, vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo(),
  bool generateMipmaps = true);

TexturePtr createBRDFLuT(DevicePtr const& device, int32_t size);

void updateMipmaps(DevicePtr const& device, TexturePtr const& texture);

} // namespace Illusion::Graphics::TextureUtils

#endif // ILLUSION_GRAPHICS_TEXTURE_UTILS_HPP
