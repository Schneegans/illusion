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

#include "Device.hpp"

namespace Illusion::Graphics::TextureUtils {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This namespace contains several higher-level utility methods for texture handling.             //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns the maximum mipmap level of a texture of the given size.
uint32_t getMaxMipmapLevels(uint32_t width, uint32_t height);

// This method will first try to load the given file with gli (DDS file format) and if that is
// impossible it will try using stb. If the file does not contain mipmaps and generateMipmaps is set
// to true, all mipmap levels will be created with linearly filtered blits.
TexturePtr createFromFile(DevicePtr const& device, std::string const& fileName,
    vk::SamplerCreateInfo samplerInfo = Device::createSamplerInfo(), bool generateMipmaps = true,
    vk::ComponentMapping const& componentMapping = vk::ComponentMapping());

// This will create a cubemap from an equirectangular panorama image. For example, you can directly
// use the images from https://hdrihaven.com/
// This is done with a compute shader.
TexturePtr createCubemapFrom360PanoramaFile(DevicePtr const& device, std::string const& fileName,
    uint32_t size, vk::SamplerCreateInfo samplerInfo = Device::createSamplerInfo(),
    bool generateMipmaps = true);

// Given an HDR cubemap with mipmaps enabled, this will create an irradiance cubemap with the given
// size as required for physically based shading.
// This is done with a compute shader.
TexturePtr createPrefilteredIrradianceCubemap(
    DevicePtr const& device, uint32_t size, TexturePtr const& inputCubemap);

// Given an HDR cubemap with mipmaps enabled, this will create a reflectance cubemap with the given
// size as required for physically based shading.
// This is done with a compute shader.
TexturePtr createPrefilteredReflectionCubemap(
    DevicePtr const& device, uint32_t size, TexturePtr const& inputCubemap);

// This generates the BRDFLuT for physically based shading with the given size.
// This is done with a compute shader.
TexturePtr createBRDFLuT(DevicePtr const& device, uint32_t size);

// Regenerates all mipmap levels of the given texture.
// This is done with linearly filtered image blits.
void updateMipmaps(DevicePtr const& device, TexturePtr const& texture);

} // namespace Illusion::Graphics::TextureUtils

#endif // ILLUSION_GRAPHICS_TEXTURE_UTILS_HPP
