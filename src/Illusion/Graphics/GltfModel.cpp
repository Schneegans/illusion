////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GltfModel.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "Texture.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include <tiny_gltf.h>

#include <functional>
#include <unordered_set>
#include <utility>

namespace Illusion::Graphics::Gltf {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Some parts of this code are inspired by Sasha Willem's GLTF loading example:                   //
// https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/base/VulkanglTFModel.hpp          //
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Filter convertFilter(int value) {
  switch (value) {
  case TINYGLTF_TEXTURE_FILTER_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
    return vk::Filter::eNearest;
  case TINYGLTF_TEXTURE_FILTER_LINEAR:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
    return vk::Filter::eLinear;
  }

  throw std::runtime_error("Invalid filter mode " + std::to_string(value));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerMipmapMode convertSamplerMipmapMode(int value) {
  switch (value) {
  case TINYGLTF_TEXTURE_FILTER_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
    return vk::SamplerMipmapMode::eNearest;
  case TINYGLTF_TEXTURE_FILTER_LINEAR:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
    return vk::SamplerMipmapMode::eLinear;
  }

  throw std::runtime_error("Invalid sampler mipmap mode " + std::to_string(value));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerAddressMode convertSamplerAddressMode(int value) {
  switch (value) {
  case TINYGLTF_TEXTURE_WRAP_REPEAT:
    return vk::SamplerAddressMode::eRepeat;
  case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
    return vk::SamplerAddressMode::eClampToEdge;
  case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
    return vk::SamplerAddressMode::eMirroredRepeat;
  }

  throw std::runtime_error("Invalid sampler address mode " + std::to_string(value));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PrimitiveTopology convertPrimitiveTopology(int value) {
  switch (value) {
  case TINYGLTF_MODE_POINTS:
    return vk::PrimitiveTopology::ePointList;
  case TINYGLTF_MODE_LINE:
    return vk::PrimitiveTopology::eLineStrip;
  case TINYGLTF_MODE_TRIANGLES:
    return vk::PrimitiveTopology::eTriangleList;
  case TINYGLTF_MODE_TRIANGLE_STRIP:
    return vk::PrimitiveTopology::eTriangleStrip;
  case TINYGLTF_MODE_TRIANGLE_FAN:
    return vk::PrimitiveTopology::eTriangleFan;
  }

  throw std::runtime_error("Invalid primitive topology " + std::to_string(value));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

Model::Model(
    std::string const& name, DevicePtr device, std::string const& file, const LoadOptions& options)
    : Core::NamedObject(name)
    , mDevice(std::move(device))
    , mRootNode(std::make_shared<Node>()) {

  // load the file ---------------------------------------------------------------------------------
  tinygltf::Model model;
  {
    std::string        extension{file.substr(file.find_last_of('.'))};
    std::string        error, warn;
    bool               success = false;
    tinygltf::TinyGLTF loader;

    if (extension == ".glb") {
      success = loader.LoadBinaryFromFile(&model, &error, &warn, file);
    } else if (extension == ".gltf") {
      success = loader.LoadASCIIFromFile(&model, &error, &warn, file);
    } else {
      throw std::runtime_error(
          "Error loading GLTF file " + file + ": Unknown extension " + extension);
    }

    if (!error.empty()) {
      throw std::runtime_error("Error loading GLTF file " + file + ": " + error);
    }
    if (!success) {
      throw std::runtime_error("Error loading GLTF file " + file);
    }
  }

  // create textures -------------------------------------------------------------------------------
  {
    if (options & LoadOptionBits::eTextures) {
      for (size_t i(0); i < model.textures.size(); ++i) {

        tinygltf::Sampler sampler;

        if (model.textures[i].sampler >= 0) {
          sampler = model.samplers[model.textures[i].sampler];
        } else {
          sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
          sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
          sampler.wrapS     = TINYGLTF_TEXTURE_WRAP_REPEAT;
          sampler.wrapT     = TINYGLTF_TEXTURE_WRAP_REPEAT;
        }

        tinygltf::Image image;

        if (model.textures[i].source >= 0) {
          image = model.images[model.textures[i].source];
        } else {
          throw std::runtime_error("Error loading GLTF file " + file + ": No image source given");
        }

        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.magFilter               = convertFilter(sampler.magFilter);
        samplerInfo.minFilter               = convertFilter(sampler.minFilter);
        samplerInfo.addressModeU            = convertSamplerAddressMode(sampler.wrapS);
        samplerInfo.addressModeV            = convertSamplerAddressMode(sampler.wrapT);
        samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable        = 1u;
        samplerInfo.maxAnisotropy           = 16;
        samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = 0u;
        samplerInfo.compareEnable           = 0u;
        samplerInfo.compareOp               = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode              = convertSamplerMipmapMode(sampler.minFilter);
        samplerInfo.mipLodBias              = 0.f;
        samplerInfo.minLod                  = 0.f;
        samplerInfo.maxLod =
            static_cast<float>(Texture::getMaxMipmapLevels(image.width, image.height));

        // TODO(simon): if no image data has been loaded, try loading it on our own
        if (image.image.empty()) {
          throw std::runtime_error(
              "Failed to load GLTF model: Non-tinygltf texture loading is not implemented yet!");
        }

        // if there is image data, create an appropriate texture object for it
        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.format =
            image.component == 3 ? vk::Format::eR8G8B8Unorm : vk::Format::eR8G8B8A8Unorm;
        imageInfo.extent.width  = image.width;
        imageInfo.extent.height = image.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = static_cast<uint32_t>(samplerInfo.maxLod);
        imageInfo.arrayLayers   = 1;
        imageInfo.samples       = vk::SampleCountFlagBits::e1;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
                          vk::ImageUsageFlagBits::eTransferDst;
        imageInfo.sharingMode   = vk::SharingMode::eExclusive;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;

        // create the texture
        auto texture = mDevice->createTexture("Texture " + std::to_string(i) + " of " + getName(),
            imageInfo, samplerInfo, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor,
            vk::ImageLayout::eShaderReadOnlyOptimal, vk::ComponentMapping(), image.image.size(),
            reinterpret_cast<void*>(image.image.data()));

        Texture::updateMipmaps(mDevice, texture);

        mTextures.push_back(texture);
      }
    }
  }

  // create materials ------------------------------------------------------------------------------
  {
    // create default material if necessary
    if (model.materials.empty()) {
      auto m = std::make_shared<Material>();

      m->mAlbedoTexture            = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mMetallicRoughnessTexture = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mNormalTexture            = mDevice->getSinglePixelTexture({127, 127, 255, 255});
      m->mOcclusionTexture         = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mEmissiveTexture          = mDevice->getSinglePixelTexture({255, 255, 255, 255});

      m->mAlbedoFactor              = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
      m->mMetallicRoughnessFactor.g = 1.f;
      m->mMetallicRoughnessFactor.b = 0.f;
      m->mDoubleSided               = true;

      mMaterials.emplace_back(m);

    } else {

      for (auto const& material : model.materials) {

        auto m = std::make_shared<Material>();

        m->mAlbedoTexture            = mDevice->getSinglePixelTexture({255, 255, 255, 255});
        m->mMetallicRoughnessTexture = mDevice->getSinglePixelTexture({255, 255, 255, 255});
        m->mNormalTexture            = mDevice->getSinglePixelTexture({127, 127, 255, 255});
        m->mOcclusionTexture         = mDevice->getSinglePixelTexture({255, 255, 255, 255});
        m->mEmissiveTexture          = mDevice->getSinglePixelTexture({255, 255, 255, 255});

        m->mName = material.name;

        for (auto const& p : material.values) {
          if (p.first == "baseColorTexture" &&
              static_cast<size_t>(p.second.TextureIndex()) < mTextures.size()) {
            m->mAlbedoTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "metallicRoughnessTexture" &&
                     static_cast<size_t>(p.second.TextureIndex()) < mTextures.size()) {
            m->mMetallicRoughnessTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "metallicFactor") {
            m->mMetallicRoughnessFactor.b = static_cast<float>(p.second.Factor());
          } else if (p.first == "roughnessFactor") {
            m->mMetallicRoughnessFactor.g = static_cast<float>(p.second.Factor());
          } else if (p.first == "baseColorFactor") {
            auto fac         = p.second.ColorFactor();
            m->mAlbedoFactor = glm::vec4(fac[0], fac[1], fac[2], fac[3]);
          } else {
            Core::Logger::warning() << "Ignoring GLTF property \"" << p.first << "\" of material "
                                    << m->mName << "\"!" << std::endl;
          }
        }

        for (auto const& p : material.extensions) {
          if (p.first == "KHR_materials_pbrSpecularGlossiness") {
            m->mSpecularGlossinessWorkflow = true;

            auto val = p.second.Get("specularGlossinessTexture");
            if (val.IsObject() &&
                static_cast<size_t>(val.Get("index").Get<int>()) < mTextures.size()) {
              m->mMetallicRoughnessTexture = mTextures[val.Get("index").Get<int>()];
            }

            val = p.second.Get("diffuseTexture");
            if (val.IsObject() &&
                static_cast<size_t>(val.Get("index").Get<int>()) < mTextures.size()) {
              m->mAlbedoTexture = mTextures[val.Get("index").Get<int>()];
            }

            val = p.second.Get("diffuseFactor");
            if (val.IsArray()) {
              m->mAlbedoFactor.r = static_cast<float>(
                  val.Get(0).IsInt() ? val.Get(0).Get<int>() : val.Get(0).Get<double>());
              m->mAlbedoFactor.g = static_cast<float>(
                  val.Get(1).IsInt() ? val.Get(1).Get<int>() : val.Get(1).Get<double>());
              m->mAlbedoFactor.b = static_cast<float>(
                  val.Get(2).IsInt() ? val.Get(2).Get<int>() : val.Get(2).Get<double>());
              m->mAlbedoFactor.a = static_cast<float>(
                  val.Get(3).IsInt() ? val.Get(3).Get<int>() : val.Get(3).Get<double>());
            }

            val = p.second.Get("specularFactor");
            if (val.IsArray()) {
              m->mMetallicRoughnessFactor.r = static_cast<float>(
                  val.Get(0).IsInt() ? val.Get(0).Get<int>() : val.Get(0).Get<double>());
              m->mMetallicRoughnessFactor.g = static_cast<float>(
                  val.Get(1).IsInt() ? val.Get(1).Get<int>() : val.Get(1).Get<double>());
              m->mMetallicRoughnessFactor.b = static_cast<float>(
                  val.Get(2).IsInt() ? val.Get(2).Get<int>() : val.Get(2).Get<double>());
            }
          } else {
            Core::Logger::warning() << "Ignoring GLTF extension \"" << p.first << "\" of material "
                                    << m->mName << "\"!" << std::endl;
          }
        }

        bool ignoreCutoff = false;
        bool hasBlendMode = false;

        for (auto const& p : material.additionalValues) {
          if (p.first == "normalTexture" &&
              static_cast<size_t>(p.second.TextureIndex()) < mTextures.size()) {
            m->mNormalTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "occlusionTexture" &&
                     static_cast<size_t>(p.second.TextureIndex()) < mTextures.size()) {
            m->mOcclusionTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "emissiveTexture" &&
                     static_cast<size_t>(p.second.TextureIndex()) < mTextures.size()) {
            m->mEmissiveTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "normalScale") {
            m->mNormalScale = static_cast<float>(p.second.Factor());
          } else if (p.first == "alphaCutoff") {
            if (!ignoreCutoff) {
              m->mAlphaCutoff = static_cast<float>(p.second.Factor());
            }
          } else if (p.first == "occlusionStrength") {
            m->mOcclusionStrength = static_cast<float>(p.second.Factor());
          } else if (p.first == "emissiveFactor") {
            auto fac           = p.second.ColorFactor();
            m->mEmissiveFactor = glm::vec3(fac[0], fac[1], fac[2]);
          } else if (p.first == "alphaMode") {
            hasBlendMode = true;
            if (p.second.string_value == "BLEND") {
              m->mDoAlphaBlending = true;
              m->mAlphaCutoff     = 0.f;
              ignoreCutoff        = true;
            } else if (p.second.string_value == "MASK") {
              m->mDoAlphaBlending = false;
            } else {
              m->mDoAlphaBlending = false;
              m->mAlphaCutoff     = 1.f;
              ignoreCutoff        = true;
            }
          } else if (p.first == "doubleSided") {
            m->mDoubleSided = p.second.bool_value;
          } else if (p.first == "name") {
            // tinygltf already loaded the name
          } else {
            Core::Logger::warning() << "Ignoring GLTF property \"" << p.first << "\" of material \""
                                    << m->mName << "\"!" << std::endl;
          }
        }

        if (!hasBlendMode) {
          m->mAlphaCutoff = 0.f;
        }

        mMaterials.emplace_back(m);
      }
    }
  }

  // create meshes & primitives --------------------------------------------------------------------
  {
    std::vector<Vertex>   vertexBuffer;
    std::vector<uint32_t> indexBuffer;

    for (auto const& m : model.meshes) {

      auto mesh   = std::make_shared<Mesh>();
      mesh->mName = m.name;

      for (auto const& p : m.primitives) {
        Primitive primitve;

        // use default material if p.material < 0
        primitve.mMaterial = mMaterials[std::max(0, p.material)];
        primitve.mTopology = convertPrimitiveTopology(p.mode);

        size_t vertexStart = vertexBuffer.size();

        auto positions = p.attributes.find("POSITION");
        if (positions == p.attributes.end()) {
          throw std::runtime_error("Failed to load GLTF model: Primitve has no vertex data!");
        }

        size_t vertexCount = model.accessors[positions->second].count;
        vertexBuffer.resize(vertexStart + vertexCount);

        // positions
        {
          auto const& a = model.accessors[positions->second];
          auto const& v = model.bufferViews[a.bufferView];

          if (a.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
            throw std::runtime_error(
                "Failed to load GLTF model: Unsupported component type for positions!");
          }

          size_t s = v.byteStride == 0 ? sizeof(glm::vec3) : v.byteStride;
          for (size_t i(0); i < vertexCount; ++i) {
            vertexBuffer[vertexStart + i].mPosition = *reinterpret_cast<glm::vec3*>(
                &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]));
            primitve.mBoundingBox.add(vertexBuffer[vertexStart + i].mPosition);
          }
        }

        auto normals = p.attributes.find("NORMAL");
        if (normals != p.attributes.end()) {
          primitve.mVertexAttributes |= Primitive::VertexAttributeBits::eNormals;

          auto const& a = model.accessors[normals->second];
          auto const& v = model.bufferViews[a.bufferView];

          if (a.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
            throw std::runtime_error(
                "Failed to load GLTF model: Unsupported component type for normals!");
          }

          size_t s = v.byteStride == 0 ? sizeof(glm::vec3) : v.byteStride;
          for (size_t i(0); i < vertexCount; ++i) {
            vertexBuffer[vertexStart + i].mNormal = *reinterpret_cast<glm::vec3*>(
                &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]));
          }
        }

        auto texcoords = p.attributes.find("TEXCOORD_0");
        if (texcoords != p.attributes.end()) {
          primitve.mVertexAttributes |= Primitive::VertexAttributeBits::eTexcoords;

          auto const& a = model.accessors[texcoords->second];
          auto const& v = model.bufferViews[a.bufferView];

          switch (a.componentType) {
          case TINYGLTF_COMPONENT_TYPE_FLOAT: {
            size_t s = v.byteStride == 0 ? sizeof(glm::vec2) : v.byteStride;
            for (size_t i(0); i < vertexCount; ++i) {
              vertexBuffer[vertexStart + i].mTexcoords = *reinterpret_cast<glm::vec2*>(
                  &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]));
            }
            break;
          }
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            size_t s = v.byteStride == 0 ? sizeof(glm::u8vec2) : v.byteStride;
            for (size_t i(0); i < vertexCount; ++i) {
              vertexBuffer[vertexStart + i].mTexcoords =
                  glm::vec2(*reinterpret_cast<glm::u8vec2*>(
                      &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]))) /
                  255.f;
            }
            break;
          }
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            size_t s = v.byteStride == 0 ? sizeof(glm::u16vec2) : v.byteStride;
            for (size_t i(0); i < vertexCount; ++i) {
              vertexBuffer[vertexStart + i].mTexcoords =
                  glm::vec2(*reinterpret_cast<glm::u16vec2*>(
                      &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]))) /
                  65535.f;
            }
            break;
          }
          default:
            throw std::runtime_error(
                "Failed to load GLTF model: Unsupported component type for texcoords!");
          }
        }

        auto joints  = p.attributes.find("JOINTS_0");
        auto weights = p.attributes.find("WEIGHTS_0");

        if (joints != p.attributes.end() && weights != p.attributes.end() &&
            (options & LoadOptionBits::eSkins)) {
          primitve.mVertexAttributes |= Primitive::VertexAttributeBits::eSkins;

          {
            auto const& a = model.accessors[joints->second];
            auto const& v = model.bufferViews[a.bufferView];

            switch (a.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
              size_t s = v.byteStride == 0 ? sizeof(glm::u8vec4) : v.byteStride;
              for (size_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mJoint0 = glm::vec4(*reinterpret_cast<glm::u8vec4*>(
                    &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s])));
              }
              break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
              size_t s = v.byteStride == 0 ? sizeof(glm::u16vec4) : v.byteStride;
              for (size_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mJoint0 = glm::vec4(*reinterpret_cast<glm::u16vec4*>(
                    &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s])));
              }
              break;
            }
            default:
              throw std::runtime_error(
                  "Failed to load GLTF model: Unsupported component type for joints!");
            }
          }

          {
            auto const& a = model.accessors[weights->second];
            auto const& v = model.bufferViews[a.bufferView];

            switch (a.componentType) {
            case TINYGLTF_COMPONENT_TYPE_FLOAT: {
              size_t s = v.byteStride == 0 ? sizeof(glm::vec4) : v.byteStride;
              for (size_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mWeight0 = *reinterpret_cast<glm::vec4*>(
                    &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]));
              }
              break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
              size_t s = v.byteStride == 0 ? sizeof(glm::u8vec4) : v.byteStride;
              for (size_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mWeight0 =
                    glm::vec4(*reinterpret_cast<glm::u8vec4*>(
                        &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]))) /
                    255.f;
              }
              break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
              size_t s = v.byteStride == 0 ? sizeof(glm::u16vec4) : v.byteStride;
              for (size_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mWeight0 =
                    glm::vec4(*reinterpret_cast<glm::u16vec4*>(
                        &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]))) /
                    65535.f;
              }
              break;
            }
            default:
              throw std::runtime_error(
                  "Failed to load GLTF model: Unsupported component type for weights!");
            }

            // normalize weights - is this the correct way of handling cases where the sum of the
            // weights is not equal to one?
            for (size_t i(0); i < vertexCount; ++i) {
              float sum = vertexBuffer[vertexStart + i].mWeight0.x +
                          vertexBuffer[vertexStart + i].mWeight0.y +
                          vertexBuffer[vertexStart + i].mWeight0.z +
                          vertexBuffer[vertexStart + i].mWeight0.w;
              if (sum > 0) {
                vertexBuffer[vertexStart + i].mWeight0 /= sum;
              }
            }
          }
        }

        if (p.indices < 0) {

          // add artificial indices if there are none
          primitve.mIndexOffset = static_cast<uint32_t>(indexBuffer.size());
          primitve.mIndexCount  = vertexCount;

          for (size_t i(0); i < vertexCount; ++i) {
            indexBuffer.push_back(i + vertexStart);
          }

        } else {

          auto const& a = model.accessors[p.indices];
          auto const& v = model.bufferViews[a.bufferView];

          primitve.mIndexOffset = static_cast<uint32_t>(indexBuffer.size());
          primitve.mIndexCount  = static_cast<uint32_t>(a.count);

          switch (a.componentType) {
          case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
            auto data = reinterpret_cast<const uint32_t*>(
                &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
            for (uint32_t i = 0; i < primitve.mIndexCount; ++i) {
              indexBuffer.push_back(data[i] + vertexStart);
            }
            break;
          }
          case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
            auto data = reinterpret_cast<const uint16_t*>(
                &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
            for (uint32_t i = 0; i < primitve.mIndexCount; ++i) {
              indexBuffer.push_back(data[i] + vertexStart);
            }
            break;
          }
          case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
            auto data = reinterpret_cast<const uint8_t*>(
                &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
            for (uint32_t i = 0; i < primitve.mIndexCount; ++i) {
              indexBuffer.push_back(data[i] + vertexStart);
            }
            break;
          }
          default:
            throw std::runtime_error("Failed to load GLTF model: Unsupported index type!");
          }
        }

        mesh->mBoundingBox.add(primitve.mBoundingBox);
        mesh->mPrimitives.emplace_back(primitve);
      }
      mMeshes.emplace_back(mesh);
    }

    mVertexBuffer = mDevice->createVertexBuffer("VertexBuffer of " + getName(), vertexBuffer);
    mIndexBuffer  = mDevice->createIndexBuffer("IndexBuffer of " + getName(), indexBuffer);
  }

  // pre-create nodes (they are referenced by themselves as children and by the skins) -------------
  for (auto const& n : model.nodes) {
    mNodes.emplace_back(std::make_shared<Node>());
  }

  // create skins ----------------------------------------------------------------------------------
  if (options & LoadOptionBits::eSkins) {
    for (auto const& s : model.skins) {
      auto skin   = std::make_shared<Skin>();
      skin->mName = s.name;

      for (int j : s.joints) {
        if (j >= 0 && static_cast<size_t>(j) < mNodes.size()) {
          skin->mJoints.push_back(mNodes[j]);
        }
      }

      if (s.inverseBindMatrices > -1) {
        auto const& a = model.accessors[s.inverseBindMatrices];
        auto const& v = model.bufferViews[a.bufferView];
        skin->mInverseBindMatrices.resize(a.count);
        std::memcpy(skin->mInverseBindMatrices.data(),
            &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset],
            a.count * sizeof(glm::mat4));
      }

      mSkins.emplace_back(skin);
    }
  }

  // create nodes ----------------------------------------------------------------------------------
  for (size_t i(0); i < model.nodes.size(); ++i) {
    mNodes[i]->mName = model.nodes[i].name;

    if (!model.nodes[i].matrix.empty()) {
      mNodes[i]->mTransform = glm::make_mat4(model.nodes[i].matrix.data());
    } else {
      if (!model.nodes[i].translation.empty()) {
        mNodes[i]->mRestTranslation = glm::make_vec3(model.nodes[i].translation.data());
        mNodes[i]->mTranslation     = mNodes[i]->mRestTranslation;
      }
      if (!model.nodes[i].rotation.empty()) {
        mNodes[i]->mRestRotation = glm::make_quat(model.nodes[i].rotation.data());
        mNodes[i]->mRotation     = mNodes[i]->mRestRotation;
      }
      if (!model.nodes[i].scale.empty()) {
        mNodes[i]->mRestScale = glm::make_vec3(model.nodes[i].scale.data());
        mNodes[i]->mScale     = mNodes[i]->mRestScale;
      }
    }

    if (model.nodes[i].mesh >= 0) {
      mNodes[i]->mMesh = mMeshes[model.nodes[i].mesh];
    }

    if (model.nodes[i].skin >= 0 && (options & LoadOptionBits::eSkins)) {
      mNodes[i]->mSkin = mSkins[model.nodes[i].skin];
    }

    for (auto c : model.nodes[i].children) {
      mNodes[i]->mChildren.push_back(mNodes[c]);
    }
  }

  for (auto i : model.scenes[std::max(0, model.defaultScene)].nodes) {
    mRootNode->mChildren.push_back(mNodes[i]);
  }

  // find root nodes of skins ----------------------------------------------------------------------
  std::function<void(NodePtr)> visit = [&visit](NodePtr const& node) {
    if (node->mSkin && !node->mSkin->mRoot) {
      node->mSkin->mRoot = node;
    }

    for (const auto& child : node->mChildren) {
      visit(child);
    }
  };

  visit(mRootNode);

  // create animations -----------------------------------------------------------------------------
  if (options & LoadOptionBits::eAnimations) {
    for (auto const& a : model.animations) {
      auto animation   = std::make_shared<Animation>();
      animation->mName = a.name;

      // Samplers
      for (auto& source : a.samplers) {
        Animation::Sampler sampler;

        if (source.interpolation == "LINEAR") {
          sampler.mType = Animation::Sampler::Type::eLinear;
        } else if (source.interpolation == "STEP") {
          sampler.mType = Animation::Sampler::Type::eStep;
        } else if (source.interpolation == "CUBICSPLINE") {
          sampler.mType = Animation::Sampler::Type::eCubicSpline;
        } else {
          Core::Logger::warning() << "Ignoring unknown animation interpolation type \""
                                  << source.interpolation << "\" for GLTF model \"" << file << "\"."
                                  << std::endl;
          sampler.mType = Animation::Sampler::Type::eLinear;
        }

        // Read sampler input time values
        {
          auto const& a = model.accessors[source.input];
          auto const& v = model.bufferViews[a.bufferView];

          auto data = reinterpret_cast<const float*>(
              &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);

          for (size_t i(0); i < a.count; ++i) {
            sampler.mKeyFrames.push_back(data[i]);
            animation->mStart = std::min(animation->mStart, data[i]);
            animation->mEnd   = std::max(animation->mEnd, data[i]);
          }
        }

        // Read sampler output T/R/S values
        {
          auto const& a = model.accessors[source.output];
          auto const& v = model.bufferViews[a.bufferView];

          if (a.type == TINYGLTF_TYPE_SCALAR) {
            auto data = reinterpret_cast<const float*>(
                &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
            for (size_t i(0); i < a.count; ++i) {
              sampler.mValues.emplace_back(data[i], 0.f, 0.f, 0.f);
            }
          } else if (a.type == TINYGLTF_TYPE_VEC2) {
            auto data = reinterpret_cast<const glm::vec2*>(
                &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
            for (size_t i(0); i < a.count; ++i) {
              sampler.mValues.emplace_back(data[i], 0.f, 0.f);
            }
          } else if (a.type == TINYGLTF_TYPE_VEC3) {
            auto data = reinterpret_cast<const glm::vec3*>(
                &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
            for (size_t i(0); i < a.count; ++i) {
              sampler.mValues.emplace_back(data[i], 0.f);
            }
          } else if (a.type == TINYGLTF_TYPE_VEC4) {
            auto data = reinterpret_cast<const glm::vec4*>(
                &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
            for (size_t i(0); i < a.count; ++i) {
              sampler.mValues.emplace_back(data[i]);
            }
          } else {
            throw std::runtime_error("Failed to load animation: Unsupported output type \"" +
                                     std::to_string(a.type) + "\"!");
          }
        }

        animation->mSamplers.emplace_back(sampler);
      }

      // Channels
      for (auto const& source : a.channels) {
        Animation::Channel channel;

        if (source.target_path == "rotation") {
          channel.mType = Animation::Channel::Type::eRotation;
        } else if (source.target_path == "translation") {
          channel.mType = Animation::Channel::Type::eTranslation;
        } else if (source.target_path == "scale") {
          channel.mType = Animation::Channel::Type::eScale;
        } else {
          Core::Logger::warning() << "Ignoring animation path type \"" << source.target_path
                                  << "\" for GLTF model \"" << file << "\"." << std::endl;
          continue;
        }

        channel.mSamplerIndex = source.sampler;
        channel.mNode         = mNodes[source.target_node];

        if (!channel.mNode) {
          continue;
        }

        animation->mChannels.emplace_back(channel);
      }

      mAnimations.emplace_back(animation);
    }
  }

  // update all global transformations -------------------------------------------------------------
  mRootNode->update(glm::mat4(1.f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Model::setAnimationTime(uint32_t animationIndex, float time) {
  if (animationIndex >= mAnimations.size()) {
    throw std::runtime_error("Failed to update GLTF animation: No animation number \"" +
                             std::to_string(animationIndex) + "\" available!");
  }

  for (auto& channel : mAnimations[animationIndex]->mChannels) {
    auto const& sampler = mAnimations[animationIndex]->mSamplers[channel.mSamplerIndex];

    if ((sampler.mType == Animation::Sampler::Type::eCubicSpline &&
            sampler.mKeyFrames.size() * 3 != sampler.mValues.size()) ||
        (sampler.mType != Animation::Sampler::Type::eCubicSpline &&
            sampler.mKeyFrames.size() != sampler.mValues.size())) {
      Core::Logger::warning()
          << "Failed to update GLTF animation: Number of data points does not match "
             "the number of keyframes. This should not happen!"
          << std::endl;
      continue;
    }

    if (sampler.mKeyFrames.empty()) {
      Core::Logger::warning()
          << "Failed to update GLTF animation: There must be at least one key frame!" << std::endl;
      continue;
    }

    size_t s = 0;
    size_t e = 0;
    float  t = 0.f;

    if (sampler.mKeyFrames.size() == 1 || time >= sampler.mKeyFrames.back()) {
      s = e = sampler.mKeyFrames.size() - 1;
    } else if (time >= sampler.mKeyFrames.front()) {
      while (time >= sampler.mKeyFrames[e]) {
        ++e;
      }
      s = e - 1;
      t = glm::clamp(
          (time - sampler.mKeyFrames[s]) / (sampler.mKeyFrames[e] - sampler.mKeyFrames[s]), 0.f,
          1.f);
    }

    if (sampler.mType == Animation::Sampler::Type::eStep) {

      if (channel.mType == Animation::Channel::Type::eTranslation) {
        channel.mNode->mTranslation = sampler.mValues[s];
      } else if (channel.mType == Animation::Channel::Type::eScale) {
        channel.mNode->mScale = sampler.mValues[s];
      } else if (channel.mType == Animation::Channel::Type::eRotation) {
        channel.mNode->mRotation = glm::normalize(glm::make_quat(&sampler.mValues[s][0]));
      }

    } else if (sampler.mType == Animation::Sampler::Type::eLinear) {

      if (channel.mType == Animation::Channel::Type::eTranslation) {
        channel.mNode->mTranslation = glm::mix(sampler.mValues[s], sampler.mValues[e], t);
      } else if (channel.mType == Animation::Channel::Type::eScale) {
        channel.mNode->mScale = glm::mix(sampler.mValues[s], sampler.mValues[e], t);
      } else if (channel.mType == Animation::Channel::Type::eRotation) {
        glm::quat q1             = glm::make_quat(&sampler.mValues[s][0]);
        glm::quat q2             = glm::make_quat(&sampler.mValues[e][0]);
        channel.mNode->mRotation = glm::normalize(glm::slerp(q1, q2, t));
      }

    } else if (sampler.mType == Animation::Sampler::Type::eCubicSpline) {

      float     d  = sampler.mKeyFrames[e] - sampler.mKeyFrames[s];
      glm::vec4 m0 = sampler.mValues[s * 3 + 0] * d;
      glm::vec4 p0 = sampler.mValues[s * 3 + 1];
      glm::vec4 m1 = sampler.mValues[s * 3 + 2] * d;
      glm::vec4 p1 = sampler.mValues[e * 3 + 1];

      glm::vec4 spline = (2.f * t * t * t - 3.f * t * t + 1.f) * p0 +
                         (t * t * t - 2.f * t * t + t) * m0 +
                         (-2.f * t * t * t + 3.f * t * t) * p1 + (t * t * t - t * t) * m1;

      if (channel.mType == Animation::Channel::Type::eTranslation) {
        channel.mNode->mTranslation = spline;
      } else if (channel.mType == Animation::Channel::Type::eScale) {
        channel.mNode->mScale = spline;
      } else if (channel.mType == Animation::Channel::Type::eRotation) {
        channel.mNode->mRotation = glm::normalize(glm::make_quat(&spline[0]));
      }
    }
  }

  mRootNode->update(glm::mat4(1.f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

NodePtr const& Model::getRoot() const {
  return mRootNode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& Model::getIndexBuffer() const {
  return mIndexBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& Model::getVertexBuffer() const {
  return mVertexBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TexturePtr> const& Model::getTextures() const {
  return mTextures;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<MaterialPtr> const& Model::getMaterials() const {
  return mMaterials;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<MeshPtr> const& Model::getMeshes() const {
  return mMeshes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<NodePtr> const& Model::getNodes() const {
  return mNodes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<AnimationPtr> const& Model::getAnimations() const {
  return mAnimations;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<SkinPtr> const& Model::getSkins() const {
  return mSkins;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Model::printInfo() const {

  // clang-format off
  Core::Logger::message() << "Textures:" << std::endl;
  for (auto const& t : mTextures) {
    Core::Logger::message() << "  " << t << ": " << t->mImageInfo.extent.width << "x"
                     << t->mImageInfo.extent.width << ", "
                     << vk::to_string(t->mImageInfo.format) << std::endl;
  }

  Core::Logger::message() << "Materials:" << std::endl;
  for (auto const& m : mMaterials) {
    Core::Logger::message() << "  " << m << ": " << m->mName << std::endl;
    Core::Logger::message() << "    SpecularGlossinessWF:     " << m->mSpecularGlossinessWorkflow << std::endl;
    Core::Logger::message() << "    AlbedoTexture:            " << m->mAlbedoTexture << std::endl;
    Core::Logger::message() << "    MetallicRoughnessTexture: " << m->mMetallicRoughnessTexture << std::endl;
    Core::Logger::message() << "    NormalTexture:            " << m->mNormalTexture << std::endl;
    Core::Logger::message() << "    OcclusionTexture:         " << m->mOcclusionTexture << std::endl;
    Core::Logger::message() << "    EmissiveTexture:          " << m->mEmissiveTexture << std::endl;
    Core::Logger::message() << "    DoAlphaBlending:          " << m->mDoAlphaBlending << std::endl;
    Core::Logger::message() << "    DoubleSided:              " << m->mDoubleSided << std::endl;
    Core::Logger::message() << "    AlbedoFactor:             " << m->mAlbedoFactor << std::endl;
    Core::Logger::message() << "    EmissiveFactor:           " << m->mEmissiveFactor << std::endl;
    Core::Logger::message() << "    MetallicRoughnessFactor:  " << m->mMetallicRoughnessFactor << std::endl;
    Core::Logger::message() << "    NormalScale:              " << m->mNormalScale << std::endl;
    Core::Logger::message() << "    OcclusionStrength:        " << m->mOcclusionStrength << std::endl;
    Core::Logger::message() << "    AlphaCutoff:              " << m->mAlphaCutoff << std::endl;
  }

  Core::Logger::message() << "Meshes:" << std::endl;
  for (auto const& m : mMeshes) {
    Core::Logger::message() << "  " << m << ": " << m->mName << std::endl;
    Core::Logger::message() << "    BoundingBox: " << m->mBoundingBox.mMin << " - " << m->mBoundingBox.mMax << std::endl;
    Core::Logger::message() << "    Primitives:" << std::endl;
    for (auto const& p : m->mPrimitives) {
      Core::Logger::message() << "      Material: " << p.mMaterial << " Topology: " << vk::to_string(p.mTopology) 
                       << " IndexCount: " << p.mIndexCount << " IndexOffset: " << p.mIndexOffset 
                       << " BoundingBox: " << p.mBoundingBox.mMin << " - " << p.mBoundingBox.mMax << std::endl;
    }
  }

  Core::Logger::message() << "Nodes:" << std::endl;
  std::function<void(Node const&, uint32_t)> printNode = [&printNode](Node const& n, uint32_t indent) {
    Core::Logger::message() << std::string(indent, ' ') << "  " << &n << ": " << n.mName << std::endl;

    if (n.mMesh) {
      Core::Logger::message() << std::string(indent, ' ') << "    Mesh:        " << n.mMesh << std::endl;
    }

    if (!n.mChildren.empty()) {
      Core::Logger::message() << std::string(indent, ' ') << "    Children:" << std::endl;
      for (auto const& c : n.mChildren) {
        printNode(*c, indent+2);
      }
    }
  };
  
  for (auto const& c : mRootNode->mChildren) {
    printNode(*c, 0);
  }

  Core::Logger::message() << "Animations:" << std::endl;
  for (auto const& a : mAnimations) {
    Core::Logger::message() << "  " << a << ": " << a->mName << std::endl;
    Core::Logger::message() << "    Samplers: " << a->mSamplers.size() << std::endl;
    Core::Logger::message() << "    Channels: " << a->mChannels.size() << std::endl;
    Core::Logger::message() << "    Start:    " << a->mStart << std::endl;
    Core::Logger::message() << "    End:      " << a->mEnd << std::endl;
  }

  Core::Logger::message() << "Skins:" << std::endl;
  for (auto const& s : mSkins) {
    Core::Logger::message() << "  " << s << ": " << s->mName << std::endl;
    Core::Logger::message() << "    Joints:              " << s->mJoints.size() << std::endl;
    Core::Logger::message() << "    InverseBindMatrices: " << s->mInverseBindMatrices.size() << std::endl;
  }
  // clang-format on
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<vk::VertexInputBindingDescription> Model::getVertexInputBindings() {
  return {{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<vk::VertexInputAttributeDescription> Model::getVertexInputAttributes() {
  return {{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(struct Vertex, mPosition)},
      {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(struct Vertex, mNormal)},
      {2, 0, vk::Format::eR32G32Sfloat, offsetof(struct Vertex, mTexcoords)},
      {3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(struct Vertex, mJoint0)},
      {4, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(struct Vertex, mWeight0)}};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

BoundingBox BoundingBox::getTransformed(glm::mat4 const& transform) {
  BoundingBox bbox;
  bbox.add((transform * glm::vec4(mMax.x, mMax.y, mMax.z, 1.f)).xyz());
  bbox.add((transform * glm::vec4(mMax.x, mMax.y, mMin.z, 1.f)).xyz());
  bbox.add((transform * glm::vec4(mMax.x, mMin.y, mMax.z, 1.f)).xyz());
  bbox.add((transform * glm::vec4(mMax.x, mMin.y, mMin.z, 1.f)).xyz());
  bbox.add((transform * glm::vec4(mMin.x, mMax.y, mMax.z, 1.f)).xyz());
  bbox.add((transform * glm::vec4(mMin.x, mMax.y, mMin.z, 1.f)).xyz());
  bbox.add((transform * glm::vec4(mMin.x, mMin.y, mMax.z, 1.f)).xyz());
  bbox.add((transform * glm::vec4(mMin.x, mMin.y, mMin.z, 1.f)).xyz());
  return bbox;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool BoundingBox::isEmpty() const {
  return mMax == glm::vec3(std::numeric_limits<float>::lowest()) &&
         mMin == glm::vec3(std::numeric_limits<float>::max());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BoundingBox::add(glm::vec3 const& point) {
  mMin = glm::min(mMin, point);
  mMax = glm::max(mMax, point);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BoundingBox::add(BoundingBox const& box) {
  if (!box.isEmpty()) {
    add(box.mMin);
    add(box.mMax);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void Node::update(glm::mat4 parentTransform) {
  mGlobalTransform = parentTransform * getLocalTransform();

  for (auto const& c : mChildren) {
    c->update(mGlobalTransform);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::mat4 Node::getLocalTransform() const {
  glm::mat4 transform(mTransform);
  transform = glm::translate(transform, mTranslation);
  transform = glm::rotate(transform, glm::angle(mRotation), glm::axis(mRotation));
  transform = glm::scale(transform, mScale);
  return transform;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

BoundingBox Node::getBoundingBox() const {
  BoundingBox bbox;
  addMeshesToBoundingBox(bbox, glm::mat4(1.f));
  return bbox;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Node::addMeshesToBoundingBox(BoundingBox& bbox, glm::mat4 const& parentTransform) const {
  glm::mat4 transform = parentTransform * getLocalTransform();
  if (mMesh) {
    bbox.add(mMesh->mBoundingBox.getTransformed(transform));
  }
  for (auto const& c : mChildren) {
    c->addMeshesToBoundingBox(bbox, transform);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<glm::mat4> Skin::getJointMatrices() const {
  std::vector<glm::mat4> jointMatrices(mJoints.size());

  glm::mat4 inverseMeshTransform = glm::inverse(mRoot->mGlobalTransform);

  for (size_t i(0); i < mJoints.size(); i++) {
    jointMatrices[i] =
        inverseMeshTransform * mJoints[i]->mGlobalTransform * mInverseBindMatrices[i];
  }

  return jointMatrices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics::Gltf
