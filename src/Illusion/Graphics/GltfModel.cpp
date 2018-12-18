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

GltfModel::GltfModel(DevicePtr const& device, std::string const& file)
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
  std::vector<TexturePtr> textures;
  {
    for (size_t i{0}; i < model.textures.size(); ++i) {

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

      // if no image data has been loaded, try loading it on our own
      if (image.image.empty()) {
        textures.push_back(TextureUtils::createFromFile(mDevice, image.uri, samplerInfo));
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

        auto texture = mDevice->createTexture(imageInfo, samplerInfo, vk::ImageViewType::e2D,
          vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eShaderReadOnlyOptimal,
          image.image.size(), (void*)image.image.data());

        TextureUtils::updateMipmaps(mDevice, texture);

        textures.push_back(texture);
      }
    }
  }

  // create materials ------------------------------------------------------------------------------
  std::vector<std::shared_ptr<Material>> materials;
  {
    for (auto const& material : model.materials) {

      auto m = std::make_shared<Material>();

      m->mAlbedoTexture            = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mMetallicRoughnessTexture = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mNormalTexture            = mDevice->getSinglePixelTexture({127, 127, 255, 255});
      m->mOcclusionTexture         = mDevice->getSinglePixelTexture({255, 255, 255, 255});
      m->mEmissiveTexture          = mDevice->getSinglePixelTexture({0, 0, 0, 255});

      m->mName = material.name;

      for (auto const& p : material.values) {
        if (p.first == "baseColorTexture") {
          m->mAlbedoTexture = textures[p.second.TextureIndex()];
        } else if (p.first == "metallicRoughnessTexture") {
          m->mMetallicRoughnessTexture = textures[p.second.TextureIndex()];
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

      for (auto const& p : material.additionalValues) {
        if (p.first == "normalTexture") {
          m->mNormalTexture = textures[p.second.TextureIndex()];
        } else if (p.first == "occlusionTexture") {
          m->mOcclusionTexture = textures[p.second.TextureIndex()];
        } else if (p.first == "emissiveTexture") {
          m->mEmissiveTexture = textures[p.second.TextureIndex()];
        } else if (p.first == "normalScale") {
          m->mPushConstants.mNormalScale = p.second.Factor();
        } else if (p.first == "alphaCutoff") {
          m->mPushConstants.mAlphaCutoff = p.second.Factor();
        } else if (p.first == "occlusionStrength") {
          m->mPushConstants.mOcclusionStrength = p.second.Factor();
        } else if (p.first == "emissiveFactor") {
          auto fac                          = p.second.ColorFactor();
          m->mPushConstants.mEmissiveFactor = glm::vec3(fac[0], fac[1], fac[2]);
        } else if (p.first == "alphaMode") {
          if (p.second.string_value == "BLEND") {
            m->mAlphaMode = Material::AlphaMode::eBlend;
          } else if (p.second.string_value == "MASK") {
            m->mAlphaMode = Material::AlphaMode::eMask;
          } else {
            m->mAlphaMode = Material::AlphaMode::eOpaque;
          }
        } else if (p.first == "doubleSided") {
          m->mDoubleSided = p.second.bool_value;
          ILLUSION_MESSAGE << "m->mDoubleSided: " << m->mDoubleSided << std::endl;
        } else if (p.first == "name") {
          // tinygltf already loaded the name
        } else {
          ILLUSION_WARNING << "Ignoring GLTF property \"" << p.first << "\" of material \""
                           << m->mName << "\"!" << std::endl;
        }
      }

      materials.emplace_back(m);
    }
  }

  // create meshed & primitives --------------------------------------------------------------------
  std::vector<std::shared_ptr<Mesh>> meshes;
  {
    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex>   vertexBuffer;

    for (auto const& m : model.meshes) {

      auto mesh = std::make_shared<Mesh>();

      for (auto const& p : m.primitives) {
        Primitive primitve;

        primitve.mMaterial = materials[p.material];
        primitve.mTopology = convertPrimitiveTopology(p.mode);

        uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());

        // append all vertices to our vertex buffer
        const float*    vertexPositions = nullptr;
        const float*    vertexNormals   = nullptr;
        const float*    vertexTexcoords = nullptr;
        const uint16_t* vertexJoints    = nullptr;
        const float*    vertexWeights   = nullptr;
        uint32_t        vertexCount     = 0;

        auto it = p.attributes.find("POSITION");
        if (it != p.attributes.end()) {
          auto const& a   = model.accessors[it->second];
          auto const& v   = model.bufferViews[a.bufferView];
          vertexPositions = reinterpret_cast<const float*>(
            &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]));
          vertexCount = a.count;
        } else {
          throw std::runtime_error("Failed to load GLTF model: Primitve has no vertex data!");
        }

        if ((it = p.attributes.find("NORMAL")) != p.attributes.end()) {
          auto const& a = model.accessors[it->second];
          auto const& v = model.bufferViews[a.bufferView];
          vertexNormals = reinterpret_cast<const float*>(
            &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]));
        }

        if ((it = p.attributes.find("TEXCOORD_0")) != p.attributes.end()) {
          auto const& a   = model.accessors[it->second];
          auto const& v   = model.bufferViews[a.bufferView];
          vertexTexcoords = reinterpret_cast<const float*>(
            &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]));
        }

        if ((it = p.attributes.find("JOINTS_0")) != p.attributes.end()) {
          auto const& a = model.accessors[it->second];
          auto const& v = model.bufferViews[a.bufferView];
          vertexJoints  = reinterpret_cast<const uint16_t*>(
            &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]));
        }

        if ((it = p.attributes.find("WEIGHTS_0")) != p.attributes.end()) {
          auto const& a = model.accessors[it->second];
          auto const& v = model.bufferViews[a.bufferView];
          vertexWeights = reinterpret_cast<const float*>(
            &(model.buffers[v.buffer].data[a.byteOffset + v.byteOffset]));
        }

        for (uint32_t v = 0; v < vertexCount; ++v) {
          Vertex vertex;
          vertex.mPosition = glm::make_vec3(&vertexPositions[v * 3]);

          primitve.mBoundingBox.add(vertex.mPosition);

          if (vertexNormals) {
            vertex.mNormal = glm::normalize(glm::make_vec3(&vertexNormals[v * 3]));
          }
          if (vertexTexcoords) {
            vertex.mTexcoords = glm::make_vec2(&vertexTexcoords[v * 2]);
          }
          if (vertexJoints && vertexWeights) {
            vertex.mJoint0  = glm::vec4(glm::make_vec4(&vertexJoints[v * 4]));
            vertex.mWeight0 = glm::make_vec4(&vertexWeights[v * 4]);
          }

          vertexBuffer.emplace_back(vertex);
        }

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

        mesh->mPrimitives.emplace_back(primitve);
        mesh->mBoundingBox.add(primitve.mBoundingBox);
      }
      meshes.emplace_back(mesh);
    }

    mVertexBuffer = mDevice->createVertexBuffer(vertexBuffer);
    mIndexBuffer  = mDevice->createIndexBuffer(indexBuffer);
  }

  // create nodes ----------------------------------------------------------------------------------
  std::vector<std::shared_ptr<Node>> nodes;
  {
    for (auto const& n : model.nodes) {
      auto node = std::make_shared<Node>();

      node->mName = n.name;

      if (n.matrix.size() > 0) {
        node->mModelMatrix *= glm::make_mat4(n.matrix.data());
      } else {
        if (n.translation.size() > 0) {
          node->mModelMatrix =
            glm::translate(node->mModelMatrix, glm::make_vec3(n.translation.data()));
        }
        if (n.rotation.size() > 0) {
          glm::dquat quaternion(glm::make_quat(n.rotation.data()));
          node->mModelMatrix =
            glm::rotate(node->mModelMatrix, glm::angle(quaternion), glm::axis(quaternion));
        }
        if (n.scale.size() > 0) {
          node->mModelMatrix = glm::scale(node->mModelMatrix, glm::make_vec3(n.scale.data()));
        }
      }

      if (n.mesh >= 0) {
        node->mMesh = meshes[n.mesh];
        node->mBoundingBox.add(meshes[n.mesh]->mBoundingBox);
      }

      nodes.emplace_back(node);
    }
  }

  // add children to nodes -------------------------------------------------------------------------
  {
    for (size_t i(0); i < model.nodes.size(); ++i) {
      for (auto const& c : model.nodes[i].children) {
        nodes[i]->mChildren.push_back(nodes[c]);
        nodes[i]->mBoundingBox.add(nodes[c]->mBoundingBox);
      }
    }
  }

  // add children to root node ---------------------------------------------------------------------
  {
    for (int n : model.scenes[model.defaultScene].nodes) {
      mRootNode.mChildren.push_back(nodes[n]);
      mRootNode.mBoundingBox.add(nodes[n]->mBoundingBox);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<GltfModel::Node>> const& GltfModel::getNodes() const {
  return mRootNode.mChildren;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GltfModel::BoundingBox const& GltfModel::getBoundingBox() const { return mRootNode.mBoundingBox; }

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& GltfModel::getIndexBuffer() const { return mIndexBuffer; }

////////////////////////////////////////////////////////////////////////////////////////////////////

BackedBufferPtr const& GltfModel::getVertexBuffer() const { return mVertexBuffer; }

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
