////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Texture.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "ShaderProgram.hpp"

#include <gli/gli.hpp>
#include <iostream>
#include <stb_image.h>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
bool isFormatSupported(DevicePtr const& device, vk::Format format) {
  auto const& features =
    device->getPhysicalDevice()->getFormatProperties(format).optimalTilingFeatures;

  return (bool)(features & vk::FormatFeatureFlagBits::eSampledImage);
}

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createFromFile(
  DevicePtr const& device, std::string const& fileName, vk::SamplerCreateInfo const& sampler) {

  auto result = std::make_shared<Texture>();

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
      throw std::runtime_error{
        "Failed to load texture " + fileName + ": Unsuppoerted texture target!"};
    }

    vk::Format format(static_cast<vk::Format>(texture.format()));

    if (format == vk::Format::eR8G8B8Unorm && !isFormatSupported(device, format)) {
      format  = vk::Format::eR8G8B8A8Unorm;
      texture = gli::convert(gli::texture2d(texture), gli::FORMAT_RGBA8_UNORM_PACK8);
    }

    std::vector<TextureLevel> levels;
    for (uint32_t i{0}; i < texture.levels(); ++i) {
      levels.push_back({texture.extent(i).x, texture.extent(i).y, texture.size(i)});
    }

    result->initData(device, levels, format, vk::ImageUsageFlagBits::eSampled, type, sampler,
      texture.size(), texture.data());

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

    result->initData(device, levels, format, vk::ImageUsageFlagBits::eSampled,
      vk::ImageViewType::e2D, sampler, size, data);

    stbi_image_free(data);

    return result;
  }

  std::string error(stbi_failure_reason());

  throw std::runtime_error{"Failed to load texture " + fileName + ": " + error};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createCubemapFrom360PanoramaFile(DevicePtr const& device,
  std::string const& fileName, int32_t size, vk::SamplerCreateInfo const& sampler) {

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
  auto outputImage = createCubemap(device, size, vk::Format::eR32G32B32A32Sfloat,
    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
    createSampler(
      vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eRepeat));

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setTexture(panorama, 0, 0);
  cmd->bindingState().setStorageImage(outputImage, 0, 1);

  cmd->begin();
  cmd->setShaderProgram(shader);

  uint32_t groupCount = std::ceil(static_cast<float>(size) / 16.f);
  cmd->dispatch(groupCount, groupCount, 6);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  return outputImage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::create2D(DevicePtr const& device, int32_t width, int32_t height,
  vk::Format format, vk::ImageUsageFlags const& usage, vk::SamplerCreateInfo const& sampler,
  size_t dataSize, const void* data) {

  TextureLevel level;
  level.mWidth  = width;
  level.mHeight = height;
  level.mSize   = dataSize;

  auto result = std::make_shared<Texture>();
  result->initData(device, {level}, format, usage, vk::ImageViewType::e2D, sampler, dataSize, data);
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::create2DMipMap(DevicePtr const& device, std::vector<TextureLevel> levels,
  vk::Format format, vk::ImageUsageFlags const& usage, vk::ImageViewType type,
  vk::SamplerCreateInfo const& sampler, size_t dataSize, const void* data) {

  auto result = std::make_shared<Texture>();
  result->initData(device, levels, format, usage, type, sampler, dataSize, data);
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createCubemap(DevicePtr const& device, int32_t size, vk::Format format,
  vk::ImageUsageFlags const& usage, vk::SamplerCreateInfo const& sampler, size_t dataSize,
  const void* data) {

  TextureLevel level;
  level.mWidth  = size;
  level.mHeight = size;
  level.mSize   = dataSize;

  auto result = std::make_shared<Texture>();
  result->initData(
    device, {level}, format, usage, vk::ImageViewType::eCube, sampler, dataSize, data);
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createBRDFLuT(DevicePtr const& device, int32_t size) {

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

  auto outputImage = create2D(device, size, size, vk::Format::eR32G32Sfloat,
    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, createSampler());

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setStorageImage(outputImage, 0, 0);

  cmd->begin();
  cmd->setShaderProgram(shader);

  uint32_t groupCount = std::ceil(static_cast<float>(size) / 16.f);
  cmd->dispatch(groupCount, groupCount, 1);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  return outputImage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerCreateInfo Texture::createSampler(
  vk::Filter filter, vk::SamplerMipmapMode mipmapMode, vk::SamplerAddressMode addressMode) {

  return vk::SamplerCreateInfo(
    vk::SamplerCreateFlags(), filter, filter, mipmapMode, addressMode, addressMode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::Texture() { ILLUSION_TRACE << "Creating Texture." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::~Texture() { ILLUSION_TRACE << "Deleting Texture." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture::initData(DevicePtr const& device, std::vector<TextureLevel> levels, vk::Format format,
  vk::ImageUsageFlags usage, vk::ImageViewType type, vk::SamplerCreateInfo const& sampler,
  size_t size, const void* data) {

  if (data) {
    usage |= vk::ImageUsageFlagBits::eTransferDst;
  }

  uint32_t             layerCount{1u};
  vk::ImageCreateFlags flags;

  if (type == vk::ImageViewType::eCube) {
    layerCount = 6u;
    flags      = vk::ImageCreateFlagBits::eCubeCompatible;
  }

  auto image = device->createBackedImage(levels[0].mWidth, levels[0].mHeight, 1, levels.size(),
    layerCount, format, vk::ImageTiling::eOptimal, usage, vk::MemoryPropertyFlagBits::eDeviceLocal,
    vk::SampleCountFlagBits::e1, flags);

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

    mImageView = device->createImageView(info);
  }

  {
    vk::SamplerCreateInfo info(sampler);
    info.maxLod = levels.size();

    mSampler = device->createSampler(info);
  }

  vk::ImageSubresourceRange subresourceRange;
  subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount   = levels.size();
  subresourceRange.layerCount   = layerCount;

  if (data) {
    auto cmd = std::make_shared<CommandBuffer>(device);
    cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    {
      cmd->transitionImageLayout(*mImage, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer,
        subresourceRange);
    }
    cmd->end();
    cmd->submit();
    cmd->waitIdle();

    auto stagingBuffer = device->createBackedBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, data);

    cmd->reset();
    cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
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

      cmd->copyBufferToImage(
        *stagingBuffer->mBuffer, *mImage, vk::ImageLayout::eTransferDstOptimal, infos);
    }
    cmd->end();
    cmd->submit();
    cmd->waitIdle();
    cmd->reset();
    cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    {
      cmd->transitionImageLayout(*mImage, vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eTransfer,
        subresourceRange);
    }
    cmd->end();
    cmd->submit();
    cmd->waitIdle();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
