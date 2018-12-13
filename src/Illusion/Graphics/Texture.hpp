////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_TEXTURE_HPP
#define ILLUSION_GRAPHICS_TEXTURE_HPP

#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Texture {

 public:
  struct TextureLevel {
    int32_t  mWidth;
    int32_t  mHeight;
    uint64_t mSize;
  };

  static TexturePtr createFromFile(DevicePtr const& device, std::string const& fileName,
    vk::SamplerCreateInfo const& sampler = vk::SamplerCreateInfo(), bool generateMipmaps = true);

  static TexturePtr createCubemapFrom360PanoramaFile(DevicePtr const& device,
    std::string const& fileName, int32_t size,
    vk::SamplerCreateInfo const& sampler = vk::SamplerCreateInfo(), bool generateMipmaps = true);

  static TexturePtr create2D(DevicePtr const& device, int32_t width, int32_t height,
    vk::Format format, vk::ImageUsageFlags const& usage, vk::SamplerCreateInfo const& sampler,
    size_t dataSize = 0, const void* data = nullptr, bool generateMipmaps = true);

  static TexturePtr createCubemap(DevicePtr const& device, int32_t size, vk::Format format,
    vk::ImageUsageFlags const& usage, vk::SamplerCreateInfo const& sampler, size_t dataSize = 0,
    const void* data = nullptr, bool generateMipmaps = true);

  static TexturePtr createBRDFLuT(DevicePtr const& device, int32_t size);

  static vk::SamplerCreateInfo createSampler(vk::Filter filter     = vk::Filter::eLinear,
    vk::SamplerMipmapMode                               mipmapMode = vk::SamplerMipmapMode::eLinear,
    vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eClampToEdge);

  Texture();
  virtual ~Texture();

  void updateMipmaps(DevicePtr const& device);

  vk::ImageViewPtr const& getImageView() const { return mImageView; }
  vk::SamplerPtr const&   getSampler() const { return mSampler; }

 protected:
  void initData(DevicePtr const& device, std::vector<TextureLevel> levels, vk::Format format,
    vk::ImageUsageFlags usage, vk::ImageViewType type, vk::SamplerCreateInfo const& sampler,
    size_t size, const void* data, bool generateMipmaps);

  BackedImagePtr mImage;

  vk::ImageViewCreateInfo mInfo;
  vk::ImageViewPtr        mImageView;
  vk::SamplerPtr          mSampler;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_TEXTURE_HPP
