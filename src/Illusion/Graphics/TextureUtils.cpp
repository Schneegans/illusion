////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TextureUtils.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "ShaderProgram.hpp"

#include <gli/gli.hpp>
#include <iostream>
#include <stb_image.h>

namespace Illusion::Graphics::TextureUtils {

////////////////////////////////////////////////////////////////////////////////////////////////////

bool formatSupportsLinearSampling(DevicePtr const& device, vk::Format format) {
  auto const& features =
    device->getPhysicalDevice()->getFormatProperties(format).optimalTilingFeatures;

  return (bool)(features & vk::FormatFeatureFlagBits::eSampledImageFilterLinear);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t getMaxMipmapLevels(uint32_t width, uint32_t height) {
  return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr createFromFile(DevicePtr const& device, std::string const& fileName,
  vk::SamplerCreateInfo samplerInfo, bool generateMipmaps) {

  // first try loading with gli
  gli::texture texture(gli::load(fileName));
  if (!texture.empty()) {

    ILLUSION_TRACE << "Creating Texture for file " << fileName << " with gli." << std::endl;

    vk::ImageViewType type;

    if (texture.target() == gli::target::TARGET_2D) {
      type = vk::ImageViewType::e2D;
    } else if (texture.target() == gli::target::TARGET_CUBE) {
      type = vk::ImageViewType::eCube;
    } else {
      throw std::runtime_error(
        "Failed to load texture " + fileName + ": Unsupported texture target!");
    }

    vk::Format format(static_cast<vk::Format>(texture.format()));

    if (format == vk::Format::eR8G8B8Unorm && !formatSupportsLinearSampling(device, format)) {
      format  = vk::Format::eR8G8B8A8Unorm;
      texture = gli::convert(gli::texture2d(texture), gli::FORMAT_RGBA8_UNORM_PACK8);
    }

    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.format        = format;
    imageInfo.extent.width  = texture.extent().x;
    imageInfo.extent.height = texture.extent().y;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = texture.levels();
    imageInfo.arrayLayers   = texture.layers();
    imageInfo.samples       = vk::SampleCountFlagBits::e1;
    imageInfo.tiling        = vk::ImageTiling::eOptimal;
    imageInfo.usage         = vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    if (imageInfo.mipLevels > 1) {
      generateMipmaps = false;
    }

    if (generateMipmaps) {
      imageInfo.mipLevels = getMaxMipmapLevels(imageInfo.extent.width, imageInfo.extent.height);
      imageInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc;
      imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
      samplerInfo.maxLod = imageInfo.mipLevels;
    }

    auto outputImage = device->createTexture(imageInfo, samplerInfo, vk::ImageViewType::e2D,
      vk::ImageAspectFlagBits::eColor, texture.size(), texture.data());

    if (generateMipmaps) {
      updateMipmaps(device, outputImage);
    }

    return outputImage;
  }

  // then try stb_image
  int   width, height, components, bytes;
  void* data;

  if (stbi_is_hdr(fileName.c_str())) {
    ILLUSION_TRACE << "Creating HDR Texture for file " << fileName << " with stb." << std::endl;
    data  = stbi_loadf(fileName.c_str(), &width, &height, &components, 4);
    bytes = 4;
  } else {
    ILLUSION_TRACE << "Creating Texture for file " << fileName << " with stb." << std::endl;
    data  = stbi_load(fileName.c_str(), &width, &height, &components, 4);
    bytes = 1;
  }

  if (data) {
    uint64_t size = width * height * bytes * 4;

    vk::Format format;
    if (bytes == 1)
      format = vk::Format::eR8G8B8A8Unorm;
    else
      format = vk::Format::eR32G32B32A32Sfloat;

    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.format        = format;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.samples       = vk::SampleCountFlagBits::e1;
    imageInfo.tiling        = vk::ImageTiling::eOptimal;
    imageInfo.usage         = vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    if (generateMipmaps) {
      imageInfo.mipLevels = getMaxMipmapLevels(width, height);
      imageInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc;
      imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
      samplerInfo.maxLod = imageInfo.mipLevels;
    }

    auto result = device->createTexture(
      imageInfo, samplerInfo, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, size, data);

    stbi_image_free(data);

    if (generateMipmaps) {
      updateMipmaps(device, result);
    }

    return result;
  }

  std::string error(stbi_failure_reason());

  throw std::runtime_error("Failed to load texture " + fileName + ": " + error);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr createCubemapFrom360PanoramaFile(DevicePtr const& device, std::string const& fileName,
  int32_t size, vk::SamplerCreateInfo samplerInfo, bool generateMipmaps) {

  std::string glsl = R"(
    #version 450

    // inputs
    layout (local_size_x = 16, local_size_y = 16, local_size_z = 6) in;

    // outputs
    layout (binding = 0)                    uniform sampler2D inputImage;
    layout (rgba32f, binding = 1) writeonly uniform imageCube outputImage;

    // constants
    vec3 majorAxes[6] = vec3[6](
      vec3( 1,  0,  0), vec3(-1,  0,  0),
      vec3( 0,  1,  0), vec3( 0, -1,  0),
      vec3( 0,  0,  1), vec3( 0,  0, -1)
    );

    vec3 s[6] = vec3[6](
      vec3( 0,  0, -1), vec3( 0,  0,  1),
      vec3( 1,  0,  0), vec3( 1,  0,  0),
      vec3( 1,  0,  0), vec3(-1,  0,  0)
    );

    vec3 t[6] = vec3[6](
      vec3( 0, -1,  0), vec3( 0, -1,  0),
      vec3( 0,  0,  1), vec3( 0,  0, -1),
      vec3( 0, -1,  0), vec3( 0, -1,  0)
    );

    void main() {
      const uvec2 size = imageSize(outputImage);
      const uint  face = gl_GlobalInvocationID.z;

      if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y) {
          return;
      }

      const vec2 st = vec2(gl_GlobalInvocationID.xy) / size - 0.5;
      const vec3 dir = normalize(s[face] * st.s + t[face] * st.t + 0.5 * majorAxes[face]);

      const vec2 lngLat = vec2(atan(dir.x, dir.z), asin(dir.y));
      const vec2 uv = (lngLat / 3.14159265359 + vec2(0, 0.5)) * vec2(0.5, -1);

      imageStore(outputImage, ivec3(gl_GlobalInvocationID), vec4(texture(inputImage, uv).rgb, 1.0) );
    }
  )";

  auto panorama = createFromFile(device, fileName);
  auto shader = ShaderProgram::createFromGlsl(device, {{vk::ShaderStageFlagBits::eCompute, glsl}});

  vk::ImageCreateInfo imageInfo;
  imageInfo.flags         = vk::ImageCreateFlagBits::eCubeCompatible;
  imageInfo.imageType     = vk::ImageType::e2D;
  imageInfo.format        = vk::Format::eR32G32B32A32Sfloat;
  imageInfo.extent.width  = size;
  imageInfo.extent.height = size;
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = 6;
  imageInfo.samples       = vk::SampleCountFlagBits::e1;
  imageInfo.tiling        = vk::ImageTiling::eOptimal;
  imageInfo.usage         = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
  imageInfo.sharingMode   = vk::SharingMode::eExclusive;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;

  if (generateMipmaps) {
    imageInfo.mipLevels = getMaxMipmapLevels(size, size);
    imageInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc;
    imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
    samplerInfo.maxLod = imageInfo.mipLevels;
  }

  auto outputImage = device->createTexture(
    imageInfo, samplerInfo, vk::ImageViewType::eCube, vk::ImageAspectFlagBits::eColor);

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setTexture(panorama, 0, 0);
  cmd->bindingState().setStorageImage(outputImage, 0, 1);

  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmd->setShaderProgram(shader);

  uint32_t groupCount = std::ceil(static_cast<float>(size) / 16.f);
  cmd->dispatch(groupCount, groupCount, 6);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  if (generateMipmaps) {
    updateMipmaps(device, outputImage);
  }

  return outputImage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr createBRDFLuT(DevicePtr const& device, int32_t size) {

  std::string glsl = R"(
    #version 450

    // inputs
    layout (local_size_x = 16, local_size_y = 16) in;

    // outputs
    layout (rgba32f, set = 0, binding = 0) writeonly uniform image2D outputImage;

    // constants
    #define PI 3.14159265359

    // Brian Karis s2013_pbs_epic_notes_v2.pdf
    vec3 ImportanceSampleGGX( vec2 Xi, float Roughness, vec3 N) {
      float a = Roughness * Roughness;
      
      float Phi = 2 * PI * Xi.x;
      float CosTheta = sqrt( (1.0 - Xi.y) / ( 1.0 + (a*a - 1.0) * Xi.y ) );
      float SinTheta = sqrt( 1.0 - CosTheta * CosTheta );
      
      vec3 H = vec3(SinTheta * cos( Phi ), SinTheta * sin( Phi ), CosTheta);
      vec3 up = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);

      vec3 TangentX = normalize( cross( up, N ) );
      vec3 TangentY = cross( N, TangentX );
      
      // Tangent to world space
      return normalize(TangentX * H.x + TangentY * H.y + N * H.z);
    }

    // http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    float radicalInverse_VdC(uint bits) {
      bits = (bits << 16u) | (bits >> 16u);
      bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
      bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
      bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
      bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
      
      return float(bits) * 2.3283064365386963e-10;
    }

    vec2 Hammersley(uint i, uint n) { 
      return vec2(float(i)/float(n), radicalInverse_VdC(i));
    }

    // http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
    float GGX(float nDotV, float a) {
      // lipsryme, http://www.gamedev.net/topic/658769-ue4-ibl-glsl/
      // http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
      float k = a / 2.0;
      return nDotV / (nDotV * (1.0 - k) + k);
    } 

    float G_Smith(float Roughness, float nDotV, float nDotL) {
      // lipsryme, http://www.gamedev.net/topic/658769-ue4-ibl-glsl/ 
      float a = Roughness * Roughness;
      return GGX(nDotL, a) * GGX(nDotV, a);
    }

    vec2 IntegrateBRDF( float Roughness, float NoV , vec3 N) {
        vec3 V = vec3( sqrt ( 1.0 - NoV * NoV ) //sin
                     , 0.0
                     , NoV); // cos
        float A = 0.0;
        float B = 0.0;
        const uint NumSamples = 1024u;
        for ( uint i = 0u; i < NumSamples; i++ ) {
            vec2 Xi = Hammersley( i, NumSamples );
            vec3 H = ImportanceSampleGGX( Xi, Roughness, N );
            vec3 L = 2.0 * dot(V, H) * H - V;
            float NoL = clamp((L.z), 0, 1);
            float NoH = clamp((H.z), 0, 1);
            float VoH = clamp((dot(V, H)), 0, 1);
            if ( NoL > 0.0 ) {
                float G = G_Smith(Roughness, NoV, NoL);
                float G_Vis = G * VoH / (NoH * NoV);
                float Fc = pow(1.0 - VoH, 5.0);
                A += (1.0 - Fc) * G_Vis;
                B += Fc * G_Vis;
            }
        }
        return vec2(A, B) / float(NumSamples);
    }

    void main() {
      ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
      ivec2 size = imageSize(outputImage);

      if (storePos.x >= size.x || storePos.y >= size.y) {
          return;
      }

      vec2 fragCoord = vec2(storePos) + vec2(0.5);
      vec2 resolution = vec2(size);
      vec2 uv = fragCoord / resolution;

      vec3 N = vec3(0,0,1); 
      float NdotV = uv.x;
      float Roughness = uv.y;

      vec2 result = IntegrateBRDF(Roughness, NdotV, N);

      imageStore(outputImage, storePos, vec4(result, 0.0, 0.0) );
    }
  )";

  auto shader = ShaderProgram::createFromGlsl(device, {{vk::ShaderStageFlagBits::eCompute, glsl}});

  vk::ImageCreateInfo imageInfo;
  imageInfo.imageType     = vk::ImageType::e2D;
  imageInfo.format        = vk::Format::eR32G32Sfloat;
  imageInfo.extent.width  = size;
  imageInfo.extent.height = size;
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = 1;
  imageInfo.samples       = vk::SampleCountFlagBits::e1;
  imageInfo.tiling        = vk::ImageTiling::eOptimal;
  imageInfo.usage         = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
  imageInfo.sharingMode   = vk::SharingMode::eExclusive;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;

  auto outputImage = device->createTexture(imageInfo, device->createSamplerInfo(),
    vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setStorageImage(outputImage, 0, 0);

  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmd->setShaderProgram(shader);

  uint32_t groupCount = std::ceil(static_cast<float>(size) / 16.f);
  cmd->dispatch(groupCount, groupCount, 1);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  return outputImage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void updateMipmaps(DevicePtr const& device, TexturePtr const& texture) {

  if (!formatSupportsLinearSampling(device, texture->mBackedImage->mImageInfo.format)) {
    throw std::runtime_error(
      "Failed to generate mipmaps: Texture format does not support linear sampling!");
  }

  auto cmd = CommandBuffer::create(device, QueueType::eGeneric);
  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  vk::ImageSubresourceRange subresourceRange;
  subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount   = 1;
  subresourceRange.layerCount   = texture->mBackedImage->mImageInfo.arrayLayers;
  subresourceRange.baseMipLevel = 0;

  uint32_t mipWidth  = texture->mBackedImage->mImageInfo.extent.width;
  uint32_t mipHeight = texture->mBackedImage->mImageInfo.extent.height;

  cmd->transitionImageLayout(*texture->mBackedImage->mImage, texture->mBackedImage->mCurrentLayout,
    vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eFragmentShader,
    vk::PipelineStageFlagBits::eTransfer, subresourceRange);

  for (uint32_t i = 1; i < texture->mBackedImage->mImageInfo.mipLevels; ++i) {

    subresourceRange.baseMipLevel = i;

    cmd->transitionImageLayout(*texture->mBackedImage->mImage,
      texture->mBackedImage->mCurrentLayout, vk::ImageLayout::eTransferDstOptimal,
      vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer,
      subresourceRange);

    cmd->blitImage(*texture->mBackedImage->mImage, i - 1, *texture->mBackedImage->mImage, i,
      glm::uvec2(mipWidth, mipHeight),
      glm::uvec2(std::max(mipWidth / 2, 1u), std::max(mipHeight / 2, 1u)),
      subresourceRange.layerCount, vk::Filter::eLinear);

    cmd->transitionImageLayout(*texture->mBackedImage->mImage, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer, subresourceRange);

    mipWidth  = std::max(mipWidth / 2, 1u);
    mipHeight = std::max(mipHeight / 2, 1u);
  }

  subresourceRange.levelCount   = texture->mBackedImage->mImageInfo.mipLevels;
  subresourceRange.baseMipLevel = 0;

  cmd->transitionImageLayout(*texture->mBackedImage->mImage, vk::ImageLayout::eTransferSrcOptimal,
    vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eTransfer,
    vk::PipelineStageFlagBits::eFragmentShader, subresourceRange);

  texture->mBackedImage->mCurrentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

  cmd->end();
  cmd->submit();
  cmd->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics::TextureUtils
