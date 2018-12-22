////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GltfModel.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "TextureUtils.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include <functional>
#include <unordered_set>

namespace Illusion::Graphics {

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

GltfModel::GltfModel(
  DevicePtr const& device, std::string const& file, TextureChannelMapping const& textureChannels)
  : mDevice(device) {

  // load the file ---------------------------------------------------------------------------------
  tinygltf::Model model;
  {
    std::string        extension{file.substr(file.find_last_of('.'))};
    std::string        error, warn;
    bool               success = false;
    tinygltf::TinyGLTF loader;

    if (extension == ".glb") {
      ILLUSION_TRACE << "Loading binary file " << file << "..." << std::endl;
      success = loader.LoadBinaryFromFile(&model, &error, &warn, file);
    } else if (extension == ".gltf") {
      ILLUSION_TRACE << "Loading ascii file " << file << "..." << std::endl;
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
      samplerInfo.anisotropyEnable        = true;
      samplerInfo.maxAnisotropy           = 16;
      samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
      samplerInfo.unnormalizedCoordinates = false;
      samplerInfo.compareEnable           = false;
      samplerInfo.compareOp               = vk::CompareOp::eAlways;
      samplerInfo.mipmapMode              = convertSamplerMipmapMode(sampler.minFilter);
      samplerInfo.mipLodBias              = 0.f;
      samplerInfo.minLod                  = 0.f;
      samplerInfo.maxLod = TextureUtils::getMaxMipmapLevels(image.width, image.height);

      // TODO: if no image data has been loaded, try loading it on our own
      if (image.image.empty()) {
        throw std::runtime_error(
          "Failed to load GLTF model: Non-tinygltf texture loading is not implemented yet!");
      } else {
        // if there is image data, create an appropriate texture object for it
        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.format =
          image.component == 3 ? vk::Format::eR8G8B8Unorm : vk::Format::eR8G8B8A8Unorm;
        imageInfo.extent.width  = image.width;
        imageInfo.extent.height = image.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = samplerInfo.maxLod;
        imageInfo.arrayLayers   = 1;
        imageInfo.samples       = vk::SampleCountFlagBits::e1;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
                          vk::ImageUsageFlagBits::eTransferDst;
        imageInfo.sharingMode   = vk::SharingMode::eExclusive;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;

        // Check if this texture is used as occlusion or metallicRoughness texture, if so we need to
        // adapt the vk::ComponentMapping accordingly. Occlusion should always map to the the red
        // channel, roughness to green and metallic to blue.
        vk::ComponentMapping componentMapping;
        static const std::unordered_map<TextureChannelMapping::Channel, vk::ComponentSwizzle>
          convert = {{TextureChannelMapping::Channel::eRed, vk::ComponentSwizzle::eR},
            {TextureChannelMapping::Channel::eGreen, vk::ComponentSwizzle::eG},
            {TextureChannelMapping::Channel::eBlue, vk::ComponentSwizzle::eB}};

        for (auto const& material : model.materials) {
          for (auto const& p : material.values) {
            if (p.first == "metallicRoughnessTexture" && p.second.TextureIndex() == i) {
              componentMapping.g = convert.at(textureChannels.mRoughness);
              componentMapping.b = convert.at(textureChannels.mMetallic);
              break;
            }
          }
          for (auto const& p : material.additionalValues) {
            if (p.first == "occlusionTexture" && p.second.TextureIndex() == i) {
              componentMapping.r = convert.at(textureChannels.mOcclusion);
              break;
            }
          }
        }

        // create the texture
        auto texture = mDevice->createTexture(imageInfo, samplerInfo, vk::ImageViewType::e2D,
          vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eShaderReadOnlyOptimal,
          componentMapping, image.image.size(), (void*)image.image.data());

        TextureUtils::updateMipmaps(mDevice, texture);

        mTextures.push_back(texture);
      }
    }
  }

  // create materials ------------------------------------------------------------------------------
  {
    // create default material if necessary
    if (model.materials.size() == 0) {
      auto m = std::make_shared<Material>();

      m->mAlbedoTexture            = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mMetallicRoughnessTexture = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mNormalTexture            = mDevice->getSinglePixelTexture({127, 127, 255, 255});
      m->mOcclusionTexture         = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mEmissiveTexture          = mDevice->getSinglePixelTexture({255, 255, 255, 255});

      m->mPushConstants.mAlbedoFactor   = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
      m->mPushConstants.mMetallicFactor = 0.f;
      m->mDoubleSided                   = true;

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
          if (p.first == "baseColorTexture") {
            m->mAlbedoTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "metallicRoughnessTexture") {
            m->mMetallicRoughnessTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "metallicFactor") {
            m->mPushConstants.mMetallicFactor = p.second.Factor();
          } else if (p.first == "roughnessFactor") {
            m->mPushConstants.mRoughnessFactor = p.second.Factor();
          } else if (p.first == "baseColorFactor") {
            auto fac                        = p.second.ColorFactor();
            m->mPushConstants.mAlbedoFactor = glm::vec4(fac[0], fac[1], fac[2], fac[3]);
          } else {
            ILLUSION_WARNING << "Ignoring GLTF property \"" << p.first << "\" of material "
                             << m->mName << "\"!" << std::endl;
          }
        }

        bool ignoreCutoff = false;
        bool hasBlendMode = false;

        for (auto const& p : material.additionalValues) {
          if (p.first == "normalTexture") {
            m->mNormalTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "occlusionTexture") {
            m->mOcclusionTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "emissiveTexture") {
            m->mEmissiveTexture = mTextures[p.second.TextureIndex()];
          } else if (p.first == "normalScale") {
            m->mPushConstants.mNormalScale = p.second.Factor();
          } else if (p.first == "alphaCutoff") {
            if (!ignoreCutoff) {
              m->mPushConstants.mAlphaCutoff = p.second.Factor();
            }
          } else if (p.first == "occlusionStrength") {
            m->mPushConstants.mOcclusionStrength = p.second.Factor();
          } else if (p.first == "emissiveFactor") {
            auto fac                          = p.second.ColorFactor();
            m->mPushConstants.mEmissiveFactor = glm::vec3(fac[0], fac[1], fac[2]);
          } else if (p.first == "alphaMode") {
            hasBlendMode = true;
            if (p.second.string_value == "BLEND") {
              m->mDoAlphaBlending            = true;
              m->mPushConstants.mAlphaCutoff = 0.f;
              ignoreCutoff                   = true;
            } else if (p.second.string_value == "MASK") {
              m->mDoAlphaBlending = false;
            } else {
              m->mDoAlphaBlending            = false;
              m->mPushConstants.mAlphaCutoff = 1.f;
              ignoreCutoff                   = true;
            }
          } else if (p.first == "doubleSided") {
            m->mDoubleSided = p.second.bool_value;
          } else if (p.first == "name") {
            // tinygltf already loaded the name
          } else {
            ILLUSION_WARNING << "Ignoring GLTF property \"" << p.first << "\" of material \""
                             << m->mName << "\"!" << std::endl;
          }
        }

        if (!hasBlendMode) {
          m->mPushConstants.mAlphaCutoff = 0.f;
        }

        mMaterials.emplace_back(m);
      }
    }
  }

  // create meshes & primitives --------------------------------------------------------------------
  {
    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex>   vertexBuffer;

    for (auto const& m : model.meshes) {

      auto mesh   = std::make_shared<Mesh>();
      mesh->mName = m.name;

      for (auto const& p : m.primitives) {
        Primitive primitve;

        // use default material if p.material < 0
        primitve.mMaterial = mMaterials[std::max(0, p.material)];
        primitve.mTopology = convertPrimitiveTopology(p.mode);

        uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());

        auto positions = p.attributes.find("POSITION");
        if (positions == p.attributes.end()) {
          throw std::runtime_error("Failed to load GLTF model: Primitve has no vertex data!");
        }

        uint32_t vertexCount = model.accessors[positions->second].count;
        vertexBuffer.resize(vertexStart + vertexCount);

        // positions
        {
          auto const& a = model.accessors[positions->second];
          auto const& v = model.bufferViews[a.bufferView];

          if (a.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
            throw std::runtime_error(
              "Failed to load GLTF model: Unsupported component type for positions!");
          }

          uint32_t s = v.byteStride == 0 ? sizeof(glm::vec3) : v.byteStride;
          for (uint32_t i(0); i < vertexCount; ++i) {
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

          uint32_t s = v.byteStride == 0 ? sizeof(glm::vec3) : v.byteStride;
          for (uint32_t i(0); i < vertexCount; ++i) {
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
            uint32_t s = v.byteStride == 0 ? sizeof(glm::vec2) : v.byteStride;
            for (uint32_t i(0); i < vertexCount; ++i) {
              vertexBuffer[vertexStart + i].mTexcoords = *reinterpret_cast<glm::vec2*>(
                &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]));
            }
            break;
          }
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            uint32_t s = v.byteStride == 0 ? sizeof(glm::u8vec2) : v.byteStride;
            for (uint32_t i(0); i < vertexCount; ++i) {
              vertexBuffer[vertexStart + i].mTexcoords =
                glm::vec2(*reinterpret_cast<glm::u8vec2*>(
                  &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]))) /
                255.f;
            }
            break;
          }
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            uint32_t s = v.byteStride == 0 ? sizeof(glm::u16vec2) : v.byteStride;
            for (uint32_t i(0); i < vertexCount; ++i) {
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

        if (joints != p.attributes.end() && weights != p.attributes.end()) {
          primitve.mVertexAttributes |= Primitive::VertexAttributeBits::eSkins;

          {
            auto const& a = model.accessors[joints->second];
            auto const& v = model.bufferViews[a.bufferView];
            uint32_t    s = v.byteStride == 0 ? sizeof(glm::vec4) : v.byteStride;
            for (uint32_t i(0); i < vertexCount; ++i) {
              vertexBuffer[vertexStart + i].mJoint0 = *reinterpret_cast<glm::vec4*>(
                &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]));
            }
          }

          {
            auto const& a = model.accessors[weights->second];
            auto const& v = model.bufferViews[a.bufferView];

            switch (a.componentType) {
            case TINYGLTF_COMPONENT_TYPE_FLOAT: {
              uint32_t s = v.byteStride == 0 ? sizeof(glm::vec4) : v.byteStride;
              for (uint32_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mWeight0 = *reinterpret_cast<glm::vec4*>(
                  &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]));
              }
              break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
              uint32_t s = v.byteStride == 0 ? sizeof(glm::u8vec4) : v.byteStride;
              for (uint32_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mWeight0 =
                  glm::vec4(*reinterpret_cast<glm::u8vec4*>(
                    &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset + i * s]))) /
                  255.f;
              }
              break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
              uint32_t s = v.byteStride == 0 ? sizeof(glm::u16vec4) : v.byteStride;
              for (uint32_t i(0); i < vertexCount; ++i) {
                vertexBuffer[vertexStart + i].mWeight0 =
                  glm::vec4(*reinterpret_cast<glm::u16vec4*>(
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
        }

        ILLUSION_MESSAGE << "p.indices: " << p.indices << std::endl;

        // append all indices to our index buffer
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

        mesh->mBoundingBox.add(primitve.mBoundingBox);
        mesh->mPrimitives.emplace_back(primitve);
      }
      mMeshes.emplace_back(mesh);
    }

    mVertexBuffer = mDevice->createVertexBuffer(vertexBuffer);
    mIndexBuffer  = mDevice->createIndexBuffer(indexBuffer);
  }

  // create nodes ----------------------------------------------------------------------------------
  for (auto const& n : model.nodes) {
    mNodes.emplace_back(std::make_shared<Node>());
  }

  for (size_t i(0); i < model.nodes.size(); ++i) {
    mNodes[i]->mName = model.nodes[i].name;

    if (model.nodes[i].matrix.size() > 0) {
      mNodes[i]->mTransform = glm::make_mat4(model.nodes[i].matrix.data());
    } else {
      if (model.nodes[i].translation.size() > 0) {
        mNodes[i]->mRestTranslation = glm::make_vec3(model.nodes[i].translation.data());
        mNodes[i]->mTranslation     = mNodes[i]->mRestTranslation;
      }
      if (model.nodes[i].rotation.size() > 0) {
        mNodes[i]->mRestRotation = glm::make_quat(model.nodes[i].rotation.data());
        mNodes[i]->mRotation     = mNodes[i]->mRestRotation;
      }
      if (model.nodes[i].scale.size() > 0) {
        mNodes[i]->mRestScale = glm::make_vec3(model.nodes[i].scale.data());
        mNodes[i]->mScale     = mNodes[i]->mRestScale;
      }
    }

    if (model.nodes[i].mesh >= 0) {
      mNodes[i]->mMesh = mMeshes[model.nodes[i].mesh];
    }

    for (auto c : model.nodes[i].children) {
      mNodes[i]->mChildren.push_back(mNodes[c]);
    }
  }

  for (auto i : model.scenes[std::max(0, model.defaultScene)].nodes) {
    mRootNode.mChildren.push_back(mNodes[i]);
  }

  // create animations -----------------------------------------------------------------------------
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
        ILLUSION_WARNING << "Ignoring unknown animation interpolation type \""
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
            sampler.mValues.emplace_back(glm::vec4(data[i], 0.f, 0.f, 0.f));
          }
        } else if (a.type == TINYGLTF_TYPE_VEC2) {
          auto data = reinterpret_cast<const glm::vec2*>(
            &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
          for (size_t i(0); i < a.count; ++i) {
            sampler.mValues.emplace_back(glm::vec4(data[i], 0.f, 0.f));
          }
        } else if (a.type == TINYGLTF_TYPE_VEC3) {
          auto data = reinterpret_cast<const glm::vec3*>(
            &model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]);
          for (size_t i(0); i < a.count; ++i) {
            sampler.mValues.emplace_back(glm::vec4(data[i], 0.f));
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
        ILLUSION_WARNING << "Ignoring animation path type \"" << source.target_path
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

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::setAnimationTime(uint32_t animationIndex, float time) {
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
      ILLUSION_WARNING << "Failed to update GLTF animation: Number of data points does not match "
                          "the number of keyframes. This should not happen!"
                       << std::endl;
      continue;
    }

    if (sampler.mKeyFrames.size() < 2) {
      ILLUSION_WARNING << "Failed to update GLTF animation: There must be at least two key frames!"
                       << std::endl;
      continue;
    }

    size_t s = 0;
    size_t e = 0;
    float  t = 0.f;

    if (time >= sampler.mKeyFrames.back()) {
      s = e = sampler.mKeyFrames.size() - 1;
    } else if (time >= sampler.mKeyFrames.front()) {
      while (time >= sampler.mKeyFrames[e]) {
        ++e;
      }
      s = e - 1;
      t = glm::clamp(
        (time - sampler.mKeyFrames[s]) / (sampler.mKeyFrames[e] - sampler.mKeyFrames[s]), 0.f, 1.f);
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GltfModel::Node const& GltfModel::getRoot() const { return mRootNode; }

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& GltfModel::getIndexBuffer() const { return mIndexBuffer; }

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& GltfModel::getVertexBuffer() const { return mVertexBuffer; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TexturePtr> const& GltfModel::getTextures() const { return mTextures; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<GltfModel::MaterialPtr> const& GltfModel::getMaterials() const { return mMaterials; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<GltfModel::MeshPtr> const& GltfModel::getMeshes() const { return mMeshes; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<GltfModel::NodePtr> const& GltfModel::getNodes() const { return mNodes; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<GltfModel::AnimationPtr> const& GltfModel::getAnimations() const { return mAnimations; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::printInfo() const {

  // clang-format off
  ILLUSION_MESSAGE << "Textures:" << std::endl;
  for (auto const& t : mTextures) {
    ILLUSION_MESSAGE << "  " << t << ": " << t->mBackedImage->mImageInfo.extent.width << "x"
                     << t->mBackedImage->mImageInfo.extent.width << ", "
                     << vk::to_string(t->mBackedImage->mImageInfo.format) << std::endl;
  }

  ILLUSION_MESSAGE << "Materials:" << std::endl;
  for (auto const& m : mMaterials) {
    ILLUSION_MESSAGE << "  " << m << ": " << m->mName << std::endl;
    ILLUSION_MESSAGE << "    AlbedoTexture:            " << m->mAlbedoTexture << std::endl;
    ILLUSION_MESSAGE << "    MetallicRoughnessTexture: " << m->mMetallicRoughnessTexture << std::endl;
    ILLUSION_MESSAGE << "    NormalTexture:            " << m->mNormalTexture << std::endl;
    ILLUSION_MESSAGE << "    OcclusionTexture:         " << m->mOcclusionTexture << std::endl;
    ILLUSION_MESSAGE << "    EmissiveTexture:          " << m->mEmissiveTexture << std::endl;
    ILLUSION_MESSAGE << "    DoAlphaBlending:          " << m->mDoAlphaBlending << std::endl;
    ILLUSION_MESSAGE << "    DoubleSided:              " << m->mDoubleSided << std::endl;
    ILLUSION_MESSAGE << "    AlbedoFactor:             " << m->mPushConstants.mAlbedoFactor << std::endl;
    ILLUSION_MESSAGE << "    EmissiveFactor:           " << m->mPushConstants.mEmissiveFactor << std::endl;
    ILLUSION_MESSAGE << "    MetallicFactor:           " << m->mPushConstants.mMetallicFactor << std::endl;
    ILLUSION_MESSAGE << "    RoughnessFactor:          " << m->mPushConstants.mRoughnessFactor << std::endl;
    ILLUSION_MESSAGE << "    NormalScale:              " << m->mPushConstants.mNormalScale << std::endl;
    ILLUSION_MESSAGE << "    OcclusionStrength:        " << m->mPushConstants.mOcclusionStrength << std::endl;
    ILLUSION_MESSAGE << "    AlphaCutoff:              " << m->mPushConstants.mAlphaCutoff << std::endl;
  }

  ILLUSION_MESSAGE << "Meshes:" << std::endl;
  for (auto const& m : mMeshes) {
    ILLUSION_MESSAGE << "  " << m << ": " << m->mName << std::endl;
    ILLUSION_MESSAGE << "    BoundingBox: " << m->mBoundingBox.mMin << " - " << m->mBoundingBox.mMax << std::endl;
    ILLUSION_MESSAGE << "    Primitives:" << std::endl;
    for (auto const& p : m->mPrimitives) {
      ILLUSION_MESSAGE << "      Material: " << p.mMaterial << " Topology: " << vk::to_string(p.mTopology) 
                       << " IndexCount: " << p.mIndexCount << " IndexOffset: " << p.mIndexOffset 
                       << " BoundingBox: " << p.mBoundingBox.mMin << " - " << p.mBoundingBox.mMax << std::endl;
    }
  }

  ILLUSION_MESSAGE << "Nodes:" << std::endl;
  std::function<void(Node const&, uint32_t)> printNode = [&printNode](Node const& n, uint32_t indent) {
    ILLUSION_MESSAGE << std::string(indent, ' ') << "  " << &n << ": " << n.mName << std::endl;

    if (n.mMesh) {
      ILLUSION_MESSAGE << std::string(indent, ' ') << "    Mesh:        " << n.mMesh << std::endl;
    }

    if (n.mChildren.size() > 0) {
      ILLUSION_MESSAGE << std::string(indent, ' ') << "    Children:" << std::endl;
      for (auto const& c : n.mChildren) {
        printNode(*c, indent+2);
      }
    }
  };
  printNode(mRootNode, 0);

  ILLUSION_MESSAGE << "Animations:" << std::endl;
  for (auto const& a : mAnimations) {
    ILLUSION_MESSAGE << "  " << a << ": " << a->mName << std::endl;
    ILLUSION_MESSAGE << "    Samplers: " << a->mSamplers.size() << std::endl;
    ILLUSION_MESSAGE << "    Channels: " << a->mChannels.size() << std::endl;
    ILLUSION_MESSAGE << "    Start:    " << a->mStart << std::endl;
    ILLUSION_MESSAGE << "    End:      " << a->mEnd << std::endl;
  }
  // clang-format on
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<vk::VertexInputBindingDescription> GltfModel::getVertexInputBindings() {
  return {{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<vk::VertexInputAttributeDescription> GltfModel::getVertexInputAttributes() {
  return {{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(struct Vertex, mPosition)},
    {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(struct Vertex, mNormal)},
    {2, 0, vk::Format::eR32G32Sfloat, offsetof(struct Vertex, mTexcoords)},
    {3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(struct Vertex, mJoint0)},
    {4, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(struct Vertex, mWeight0)}};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
