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
#include "Shader.hpp"
#include "ShaderModule.hpp"

#include <gli/gli.hpp>
#include <iostream>
#include <stb_image.h>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

bool formatSupportsLinearSampling(DevicePtr const& device, vk::Format format) {
  auto const& features =
      device->getPhysicalDevice()->getFormatProperties(format).optimalTilingFeatures;

  return (bool)(features & vk::FormatFeatureFlagBits::eSampledImageFilterLinear);
}

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t Texture::getMaxMipmapLevels(uint32_t width, uint32_t height) {
  return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createFromFile(DevicePtr const& device, std::string const& fileName,
    vk::SamplerCreateInfo samplerInfo, bool generateMipmaps,
    vk::ComponentMapping const& componentMapping) {

  // first try loading with gli
  gli::texture texture(gli::load(fileName));
  if (!texture.empty()) {

    ILLUSION_TRACE << "Creating Texture for file " << fileName << " with gli." << std::endl;

    vk::ImageType     type;
    vk::ImageViewType viewType;

    if (texture.target() == gli::target::TARGET_2D) {
      type     = vk::ImageType::e2D;
      viewType = vk::ImageViewType::e2D;
    } else if (texture.target() == gli::target::TARGET_3D) {
      type     = vk::ImageType::e3D;
      viewType = vk::ImageViewType::e3D;
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
    imageInfo.imageType     = type;
    imageInfo.format        = format;
    imageInfo.extent.width  = texture.extent().x;
    imageInfo.extent.height = texture.extent().y;
    imageInfo.extent.depth  = texture.extent().z;
    imageInfo.mipLevels     = static_cast<uint32_t>(texture.levels());
    imageInfo.arrayLayers   = static_cast<uint32_t>(texture.layers());
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
      samplerInfo.maxLod = static_cast<float>(imageInfo.mipLevels);
    }

    auto outputImage = device->createTexture(imageInfo, samplerInfo, viewType,
        vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eShaderReadOnlyOptimal, componentMapping,
        texture.size(), texture.data());

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
      samplerInfo.maxLod = static_cast<float>(imageInfo.mipLevels);
    }

    auto result = device->createTexture(imageInfo, samplerInfo, vk::ImageViewType::e2D,
        vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eShaderReadOnlyOptimal, componentMapping,
        size, data);

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

TexturePtr Texture::createCubemapFrom360PanoramaFile(DevicePtr const& device,
    std::string const& fileName, uint32_t size, vk::SamplerCreateInfo samplerInfo,
    bool generateMipmaps) {

  std::string glsl = R"(
    #version 450

    // inputs
    layout (local_size_x = 16, local_size_y = 16, local_size_z = 6) in;

    // outputs
    layout (binding = 0)                    uniform sampler2D inputImage;
    layout (rgba32f, binding = 1) writeonly uniform imageCube outputCubemap;

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
      const uvec2 size = imageSize(outputCubemap);

      if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y) {
          return;
      }

      const uint  face = gl_GlobalInvocationID.z;
      const vec2 st = vec2(gl_GlobalInvocationID.xy) / size - 0.5;
      const vec3 dir = normalize(s[face] * st.s + t[face] * st.t + 0.5 * majorAxes[face]);

      const vec2 lngLat = vec2(atan(dir.x, dir.z), asin(dir.y));
      const vec2 uv = (lngLat / 3.14159265359 + vec2(0, -0.5)) * vec2(-0.5, -1);

      imageStore(outputCubemap, ivec3(gl_GlobalInvocationID), vec4(texture(inputImage, uv).rgb, 1.0) );
    }
  )";

  auto panorama = createFromFile(device, fileName,
      vk::SamplerCreateInfo(vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear,
          vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
          vk::SamplerAddressMode::eClampToEdge));
  auto shader   = Shader::create(device);
  shader->addModule(vk::ShaderStageFlagBits::eCompute,
      GlslCode::create(glsl, "createCubemapFrom360PanoramaFile"));

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
    samplerInfo.maxLod = static_cast<float>(imageInfo.mipLevels);
  }

  auto outputCubemap = device->createTexture(imageInfo, samplerInfo, vk::ImageViewType::eCube,
      vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral);

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setTexture(panorama, 0, 0);
  cmd->bindingState().setStorageImage(outputCubemap, 0, 1);

  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmd->setShader(shader);

  uint32_t groupCount = static_cast<uint32_t>(std::ceil(static_cast<float>(size) / 16.f));
  cmd->dispatch(groupCount, groupCount, 6);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  if (generateMipmaps) {
    updateMipmaps(device, outputCubemap);
  }

  return outputCubemap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createPrefilteredIrradianceCubemap(
    DevicePtr const& device, uint32_t size, TexturePtr const& inputCubemap) {
  std::string glsl = R"(
    #version 450

    // inputs
    layout (local_size_x = 16, local_size_y = 16, local_size_z = 6) in;

    // outputs
    layout (binding = 0)                    uniform samplerCube inputCubemap;
    layout (rgba32f, binding = 1) writeonly uniform imageCube   outputCubemap;

    // constants
    #define PI 3.14159265359

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
      const uvec2 size = imageSize(outputCubemap);

      if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y) {
          return;
      }

      const uint  face = gl_GlobalInvocationID.z;
      const vec2 st = vec2(gl_GlobalInvocationID.xy) / size - 0.5;
      const vec3 normal = normalize(s[face] * st.s + t[face] * st.t + 0.5 * majorAxes[face]);

      // from https://learnopengl.com/PBR/IBL/Diffuse-irradiance
      vec3 irradiance = vec3(0.0);

      vec3 up    = vec3(0.0, 1.0, 0.0);
      vec3 right = cross(up, normal);
      up         = cross(normal, right);
           
      float sampleDelta = 0.05;
      float nrSamples = 0.0;

      // choose an input level which we will not undersample given our sampleDelta
      float requiredSize  = 0.5 * PI / sampleDelta;
      float inputBaseSize = float(textureSize(inputCubemap, 0).x);
      float inputLevels   = float(textureQueryLevels(inputCubemap));
      float lod = clamp(log2(inputBaseSize) - log2(requiredSize), 0, inputLevels);

      for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
          // spherical to cartesian (in tangent space)
          vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
          // tangent space to world
          vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

          irradiance += (textureLod(inputCubemap, sampleVec, lod).rgb) * cos(theta) * sin(theta);

          nrSamples++;
        }
      }
      irradiance = PI * irradiance * (1.0 / float(nrSamples));

      imageStore(outputCubemap, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));
    }
  )";

  auto shader = Shader::create(device);
  shader->addModule(vk::ShaderStageFlagBits::eCompute,
      GlslCode::create(glsl, "createPrefilteredIrradianceCubemap"));

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

  auto outputCubemap = device->createTexture(imageInfo, device->createSamplerInfo(),
      vk::ImageViewType::eCube, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral);

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setTexture(inputCubemap, 0, 0);
  cmd->bindingState().setStorageImage(outputCubemap, 0, 1);

  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmd->setShader(shader);

  uint32_t groupCount = static_cast<uint32_t>(std::ceil(static_cast<float>(size) / 16.f));
  cmd->dispatch(groupCount, groupCount, 6);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  return outputCubemap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createPrefilteredReflectionCubemap(
    DevicePtr const& device, uint32_t size, TexturePtr const& inputCubemap) {
  std::string glsl = R"(
    #version 450

    // inputs
    layout (local_size_x = 16, local_size_y = 16, local_size_z = 6) in;

    // outputs
    layout (binding = 0)                    uniform samplerCube inputCubemap;
    layout (rgba32f, binding = 1) writeonly uniform imageCube   outputCubemap;

    // push constants
    layout(push_constant, std430) uniform PushConstants {
        float mCurrentLevel;
    } pushConstants;

    // constants
    #define PI 3.14159265359

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

    float DistributionGGX(vec3 N, vec3 H, float roughness) {
      float a = roughness*roughness;
      float a2 = a*a;
      float NdotH = max(dot(N, H), 0.0);
      float NdotH2 = NdotH*NdotH;

      float nom   = a2;
      float denom = (NdotH2 * (a2 - 1.0) + 1.0);
      denom = PI * denom * denom;

      return nom / denom;
    }

    // http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    // efficient VanDerCorpus calculation.
    float RadicalInverse_VdC(uint bits) {
       bits = (bits << 16u) | (bits >> 16u);
       bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
       bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
       bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
       bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
       return float(bits) * 2.3283064365386963e-10; // / 0x100000000
    }

    vec2 Hammersley(uint i, uint N) {
        return vec2(float(i)/float(N), RadicalInverse_VdC(i));
    }

    vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
      float a = roughness*roughness;
      
      float phi = 2.0 * PI * Xi.x;
      float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
      float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
      
      // from spherical coordinates to cartesian coordinates - halfway vector
      vec3 H;
      H.x = cos(phi) * sinTheta;
      H.y = sin(phi) * sinTheta;
      H.z = cosTheta;
      
      // from tangent-space H vector to world-space sample vector
      vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
      vec3 tangent   = normalize(cross(up, N));
      vec3 bitangent = cross(N, tangent);
      
      vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
      return normalize(sampleVec);
    }

    void main() {
      const uvec2 size = imageSize(outputCubemap);

      if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y) {
          return;
      }

      const uint  face      = gl_GlobalInvocationID.z;
      const float maxLevel  = float(textureQueryLevels(inputCubemap));
      const float roughness = pushConstants.mCurrentLevel / maxLevel;

      const vec2 st = vec2(gl_GlobalInvocationID.xy) / size - 0.5;
      const vec3 normal = normalize(s[face] * st.s + t[face] * st.t + 0.5 * majorAxes[face]);

      const uint SAMPLE_COUNT = 512u;
      vec3 prefilteredReflection = vec3(0.0);
      float totalWeight = 0.0;

      for(uint i = 0u; i < SAMPLE_COUNT; ++i)
      {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, normal, roughness);
        vec3 L  = normalize(2.0 * dot(normal, H) * H - normal);

        float NdotL = max(dot(normal, L), 0.0);
        if(NdotL > 0.0)
        {
          // sample from the environment's mip level based on roughness/pdf
          float D   = DistributionGGX(normal, H, roughness);
          float NdotH = max(dot(normal, H), 0.0);
          float HdotV = max(dot(H, normal), 0.0);
          float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

          float resolution = textureSize(inputCubemap, 0).x;
          float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
          float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

          float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
          
          prefilteredReflection += textureLod(inputCubemap, L, mipLevel).rgb * NdotL;
          totalWeight += NdotL;
        }
      }

      prefilteredReflection = prefilteredReflection / totalWeight;

      imageStore(outputCubemap, ivec3(gl_GlobalInvocationID), vec4(prefilteredReflection, 1.0));
    }
  )";

  auto shader = Shader::create(device);
  shader->addModule(vk::ShaderStageFlagBits::eCompute,
      GlslCode::create(glsl, "createPrefilteredReflectionCubemap"));

  uint32_t mipLevels = getMaxMipmapLevels(size, size);

  vk::ImageCreateInfo imageInfo;
  imageInfo.flags         = vk::ImageCreateFlagBits::eCubeCompatible;
  imageInfo.imageType     = vk::ImageType::e2D;
  imageInfo.format        = vk::Format::eR32G32B32A32Sfloat;
  imageInfo.extent.width  = size;
  imageInfo.extent.height = size;
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = mipLevels;
  imageInfo.arrayLayers   = 6;
  imageInfo.samples       = vk::SampleCountFlagBits::e1;
  imageInfo.tiling        = vk::ImageTiling::eOptimal;
  imageInfo.usage         = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
  imageInfo.sharingMode   = vk::SharingMode::eExclusive;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;

  auto samplerInfo   = device->createSamplerInfo();
  samplerInfo.maxLod = static_cast<float>(imageInfo.mipLevels);

  auto outputCubemap = device->createTexture(imageInfo, samplerInfo, vk::ImageViewType::eCube,
      vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral);

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setTexture(inputCubemap, 0, 0);

  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmd->setShader(shader);

  std::vector<vk::ImageViewPtr> mipViews;

  for (uint32_t i(0); i < mipLevels; ++i) {
    uint32_t groupCount = static_cast<uint32_t>(std::ceil(static_cast<float>(size) / 16.f));

    auto mipViewInfo                          = outputCubemap->mViewInfo;
    mipViewInfo.subresourceRange.baseMipLevel = i;
    mipViewInfo.subresourceRange.levelCount   = 1;
    auto mipView                              = device->createImageView(mipViewInfo);

    cmd->pushConstants((float)i);
    cmd->bindingState().setStorageImage(outputCubemap, mipView, 0, 1);
    cmd->dispatch(groupCount, groupCount, 6);

    mipViews.emplace_back(mipView);

    size = std::max(size / 2, 1u);
  }
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  return outputCubemap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Texture::createBRDFLuT(DevicePtr const& device, uint32_t size) {

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

  auto shader = Shader::create(device);
  shader->addModule(vk::ShaderStageFlagBits::eCompute, GlslCode::create(glsl, "createBRDFLuT"));

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
      vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral);

  auto cmd = CommandBuffer::create(device, QueueType::eCompute);
  cmd->bindingState().setStorageImage(outputImage, 0, 0);

  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmd->setShader(shader);

  uint32_t groupCount = static_cast<uint32_t>(std::ceil(static_cast<float>(size) / 16.f));
  cmd->dispatch(groupCount, groupCount, 1);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  return outputImage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture::updateMipmaps(DevicePtr const& device, TexturePtr const& texture) {

  if (!formatSupportsLinearSampling(device, texture->mImageInfo.format)) {
    throw std::runtime_error(
        "Failed to generate mipmaps: Texture format does not support linear sampling!");
  }

  auto cmd = CommandBuffer::create(device, QueueType::eGeneric);
  cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  vk::ImageSubresourceRange subresourceRange;
  subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount   = 1;
  subresourceRange.layerCount   = texture->mImageInfo.arrayLayers;
  subresourceRange.baseMipLevel = 0;

  uint32_t mipWidth  = texture->mImageInfo.extent.width;
  uint32_t mipHeight = texture->mImageInfo.extent.height;

  cmd->transitionImageLayout(*texture->mImage, texture->mCurrentLayout,
      vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eFragmentShader,
      vk::PipelineStageFlagBits::eTransfer, subresourceRange);

  for (uint32_t i = 1; i < texture->mImageInfo.mipLevels; ++i) {

    subresourceRange.baseMipLevel = i;

    cmd->transitionImageLayout(*texture->mImage, texture->mCurrentLayout,
        vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eFragmentShader,
        vk::PipelineStageFlagBits::eTransfer, subresourceRange);

    cmd->blitImage(*texture->mImage, i - 1, *texture->mImage, i, glm::uvec2(mipWidth, mipHeight),
        glm::uvec2(std::max(mipWidth / 2, 1u), std::max(mipHeight / 2, 1u)),
        subresourceRange.layerCount, vk::Filter::eLinear);

    cmd->transitionImageLayout(*texture->mImage, vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer, subresourceRange);

    mipWidth  = std::max(mipWidth / 2, 1u);
    mipHeight = std::max(mipHeight / 2, 1u);
  }

  subresourceRange.levelCount   = texture->mImageInfo.mipLevels;
  subresourceRange.baseMipLevel = 0;

  cmd->transitionImageLayout(*texture->mImage, vk::ImageLayout::eTransferSrcOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eFragmentShader, subresourceRange);

  texture->mCurrentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

  cmd->end();
  cmd->submit();
  cmd->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
