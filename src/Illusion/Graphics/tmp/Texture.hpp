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

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Texture {

  // ------------------------------------------------------------------------------ public interface

 public:
  struct TextureLevel {
    int32_t  mWidth;
    int32_t  mHeight;
    uint64_t mSize;
  };

  Texture() {}

  // --------------------------------------------------------------------------------------- methods
  static std::shared_ptr<Texture> createFromFile(
    std::shared_ptr<Engine> const& engine,
    std::string const&             fileName,
    vk::SamplerCreateInfo const&   sampler);

  static std::shared_ptr<Texture> create2D(
    std::shared_ptr<Engine> const& engine,
    int32_t                        width,
    int32_t                        height,
    vk::Format                     format,
    vk::ImageUsageFlags const&     usage,
    vk::SamplerCreateInfo const&   sampler,
    size_t                         size = 0,
    void*                          data = nullptr);

  static std::shared_ptr<Texture> create2DMipMap(
    std::shared_ptr<Engine> const& engine,
    std::vector<TextureLevel>      levels,
    vk::Format                     format,
    vk::ImageUsageFlags const&     usage,
    vk::ImageViewType              type,
    vk::SamplerCreateInfo const&   sampler,
    size_t                         size = 0,
    void*                          data = nullptr);

  static std::shared_ptr<Texture> createCubemap(
    std::shared_ptr<Engine> const& engine,
    int32_t                        width,
    int32_t                        height,
    vk::Format                     format,
    vk::ImageUsageFlags const&     usage,
    vk::SamplerCreateInfo const&   sampler,
    size_t                         size = 0,
    void*                          data = nullptr);

  virtual ~Texture();

  std::shared_ptr<vk::Image> const&        getImage() const { return mImage; }
  std::shared_ptr<vk::DeviceMemory> const& getMemory() const { return mMemory; }
  std::shared_ptr<vk::ImageView> const&    getImageView() const { return mImageView; }
  std::shared_ptr<vk::Sampler> const&      getSampler() const { return mSampler; }

  // --------------------------------------------------------------------------- protected interface

 protected:
  void initData(
    std::shared_ptr<Engine> const& engine,
    std::vector<TextureLevel>      levels,
    vk::Format                     format,
    vk::ImageUsageFlags            usage,
    vk::ImageViewType              type,
    vk::SamplerCreateInfo const&   sampler,
    size_t                         size,
    void*                          data);

  std::shared_ptr<vk::Image>        mImage;
  std::shared_ptr<vk::DeviceMemory> mMemory;
  std::shared_ptr<vk::ImageView>    mImageView;
  std::shared_ptr<vk::Sampler>      mSampler;
};

// -------------------------------------------------------------------------------------------------
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_TEXTURE_HPP
