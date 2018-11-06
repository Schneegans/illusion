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
#include "Texture.hpp"

#include "../Core/Logger.hpp"
#include "Engine.hpp"

#include <gli/gli.hpp>
#include <iostream>
#include <stb_image.h>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Texture> Texture::createFromFile(
  std::shared_ptr<Engine> const& engine,
  std::string const&             fileName,
  vk::SamplerCreateInfo const&   sampler) {

  auto result = std::make_shared<Texture>();

  // first try loading with gli
  gli::texture texture = gli::load(fileName);
  if (!texture.empty()) {

    ILLUSION_TRACE << "Creating Texture for file " << fileName << " with gli." << std::endl;

    std::vector<TextureLevel> levels;
    for (uint32_t i{0}; i < texture.levels(); ++i) {
      levels.push_back({texture.extent(i).x, texture.extent(i).y, texture.size(i)});
    }

    vk::ImageViewType type;

    if (texture.target() == gli::target::TARGET_2D) {
      type = vk::ImageViewType::e2D;
    } else if (texture.target() == gli::target::TARGET_CUBE) {
      type = vk::ImageViewType::eCube;
    } else {
      throw std::runtime_error{"Failed to load texture " + fileName +
                               ": Unsuppoerted texture target!"};
    }

    result->initData(
      engine,
      levels,
      (vk::Format)texture.format(),
      vk::ImageUsageFlagBits::eSampled,
      type,
      sampler,
      texture.size(),
      texture.data());

    return result;
  }

  // then try stb_image
  int   width, height, components, bytes;
  void* data;

  if (stbi_is_hdr(fileName.c_str())) {
    ILLUSION_TRACE << "Creating HDR Texture for file " << fileName << " with stb." << std::endl;
    data  = stbi_loadf(fileName.c_str(), &width, &height, &components, 0);
    bytes = 4;
  } else {
    ILLUSION_TRACE << "Creating Texture for file " << fileName << " with stb." << std::endl;
    data  = stbi_load(fileName.c_str(), &width, &height, &components, 0);
    bytes = 1;
  }

  if (data) {
    uint64_t                  size = width * height * bytes * components;
    std::vector<TextureLevel> levels;
    levels.push_back({width, height, size});

    vk::Format format;
    if (components == 1) {
      if (bytes == 1)
        format = vk::Format::eR8Unorm;
      else
        format = vk::Format::eR32Sfloat;
    } else if (components == 2) {
      if (bytes == 1)
        format = vk::Format::eR8G8Unorm;
      else
        format = vk::Format::eR32G32Sfloat;
    } else if (components == 3) {
      if (bytes == 1)
        format = vk::Format::eR8G8B8Unorm;
      else
        format = vk::Format::eR32G32B32Sfloat;
    } else {
      if (bytes == 1)
        format = vk::Format::eR8G8B8A8Unorm;
      else
        format = vk::Format::eR32G32B32A32Sfloat;
    }

    result->initData(
      engine,
      levels,
      format,
      vk::ImageUsageFlagBits::eSampled,
      vk::ImageViewType::e2D,
      sampler,
      size,
      data);

    stbi_image_free(data);

    return result;
  }

  std::string error(stbi_failure_reason());

  throw std::runtime_error{"Failed to load texture " + fileName + ": " + error};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Texture> Texture::create2D(
  std::shared_ptr<Engine> const& engine,
  int32_t                        width,
  int32_t                        height,
  vk::Format                     format,
  vk::ImageUsageFlags const&     usage,
  vk::SamplerCreateInfo const&   sampler,
  size_t                         size,
  void*                          data) {

  ILLUSION_TRACE << "Creating Texture." << std::endl;

  TextureLevel level;
  level.mWidth  = width;
  level.mHeight = height;
  level.mSize   = size;

  auto result = std::make_shared<Texture>();
  result->initData(engine, {level}, format, usage, vk::ImageViewType::e2D, sampler, size, data);
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Texture> Texture::create2DMipMap(
  std::shared_ptr<Engine> const& engine,
  std::vector<TextureLevel>      levels,
  vk::Format                     format,
  vk::ImageUsageFlags const&     usage,
  vk::ImageViewType              type,
  vk::SamplerCreateInfo const&   sampler,
  size_t                         size,
  void*                          data) {

  ILLUSION_TRACE << "Creating Texture." << std::endl;

  auto result = std::make_shared<Texture>();
  result->initData(engine, levels, format, usage, type, sampler, size, data);
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Texture> Texture::createCubemap(
  std::shared_ptr<Engine> const& engine,
  int32_t                        width,
  int32_t                        height,
  vk::Format                     format,
  vk::ImageUsageFlags const&     usage,
  vk::SamplerCreateInfo const&   sampler,
  size_t                         size,
  void*                          data) {

  ILLUSION_TRACE << "Creating Texture." << std::endl;

  TextureLevel level;
  level.mWidth  = width;
  level.mHeight = height;
  level.mSize   = size;

  auto result = std::make_shared<Texture>();
  result->initData(engine, {level}, format, usage, vk::ImageViewType::eCube, sampler, size, data);
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::~Texture() { ILLUSION_TRACE << "Deleting Texture." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture::initData(
  std::shared_ptr<Engine> const& engine,
  std::vector<TextureLevel>      levels,
  vk::Format                     format,
  vk::ImageUsageFlags            usage,
  vk::ImageViewType              type,
  vk::SamplerCreateInfo const&   sampler,
  size_t                         size,
  void*                          data) {

  if (data) { usage |= vk::ImageUsageFlagBits::eTransferDst; }

  uint32_t             layerCount{1u};
  vk::ImageCreateFlags flags;

  if (type == vk::ImageViewType::eCube) {
    layerCount = 6u;
    flags      = vk::ImageCreateFlagBits::eCubeCompatible;
  }

  auto image = engine->createBackedImage(
    levels[0].mWidth,
    levels[0].mHeight,
    1,
    levels.size(),
    layerCount,
    format,
    vk::ImageTiling::eOptimal,
    usage,
    vk::MemoryPropertyFlagBits::eDeviceLocal,
    vk::SampleCountFlagBits::e1,
    flags);

  mImage  = image->mImage;
  mMemory = image->mMemory;

  {
    vk::ImageViewCreateInfo info;
    info.image                           = *image->mImage;
    info.viewType                        = type;
    info.format                          = format;
    info.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = levels.size();
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = layerCount;

    mImageView = engine->createImageView(info);
  }

  {
    vk::SamplerCreateInfo info(sampler);
    info.maxLod = levels.size();

    mSampler = engine->createSampler(info);
  }

  vk::ImageSubresourceRange subresourceRange;
  subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount   = levels.size();
  subresourceRange.layerCount   = layerCount;

  if (data) {
    engine->transitionImageLayout(
      mImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);

    auto stagingBuffer = engine->createBackedBuffer(
      size,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
      data);

    auto cmd = engine->beginSingleTimeGraphicsCommands();
    {
      std::vector<vk::BufferImageCopy> infos;
      uint64_t                         offset = 0;

      for (uint32_t i = 0; i < levels.size(); ++i) {
        vk::BufferImageCopy info;
        info.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
        info.imageSubresource.mipLevel       = i;
        info.imageSubresource.baseArrayLayer = 0;
        info.imageSubresource.layerCount     = layerCount;
        info.imageExtent.width               = levels[i].mWidth;
        info.imageExtent.height              = levels[i].mHeight;
        info.imageExtent.depth               = 1;
        info.bufferOffset                    = offset;

        infos.push_back(info);

        offset += levels[i].mSize;
      }

      ILLUSION_TRACE << "Copying vk::Buffer to vk::Image." << std::endl;

      cmd.copyBufferToImage(
        *stagingBuffer->mBuffer, *mImage, vk::ImageLayout::eTransferDstOptimal, infos);
    }
    engine->endSingleTimeGraphicsCommands(cmd);

    engine->transitionImageLayout(
      mImage,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      subresourceRange);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Illusion::Graphics
