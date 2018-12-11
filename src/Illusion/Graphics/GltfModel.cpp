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
#include "Texture.hpp"

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
    return vk::Filter::eNearest;
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

    vk::SamplerCreateInfo info;
    info.magFilter               = convertFilter(sampler.magFilter);
    info.minFilter               = convertFilter(sampler.minFilter);
    info.addressModeU            = convertSamplerAddressMode(sampler.wrapS);
    info.addressModeV            = convertSamplerAddressMode(sampler.wrapT);
    info.addressModeW            = vk::SamplerAddressMode::eRepeat;
    info.anisotropyEnable        = true;
    info.maxAnisotropy           = 16;
    info.borderColor             = vk::BorderColor::eIntOpaqueBlack;
    info.unnormalizedCoordinates = false;
    info.compareEnable           = false;
    info.compareOp               = vk::CompareOp::eAlways;
    info.mipmapMode              = convertSamplerMipmapMode(sampler.minFilter);
    info.mipLodBias              = 0.f;
    info.minLod                  = 0.f;
    info.maxLod                  = 0.f;

    // if no image data has been loaded, try loading it on our own
    if (image.image.empty()) {
      textures.push_back(Texture::createFromFile(mDevice, image.uri, info));
    } else {
      // if there is image data, create an appropriate texture object for it
      textures.push_back(Texture::create2D(mDevice, image.width, image.height,
        image.component == 3 ? vk::Format::eR8G8B8Unorm : vk::Format::eR8G8B8A8Unorm,
        vk::ImageUsageFlagBits::eSampled, info, image.image.size(), (void*)image.image.data()));
    }
  }

  // create materials ------------------------------------------------------------------------------
  for (auto const& material : model.materials) {

    auto m = std::make_shared<Material>();

    m->mBaseColorTexture         = mDevice->getWhitePixel();
    m->mMetallicRoughnessTexture = mDevice->getWhitePixel();
    m->mNormalTexture            = mDevice->getWhitePixel();
    m->mOcclusionTexture         = mDevice->getWhitePixel();
    m->mEmissiveTexture          = mDevice->getWhitePixel();

    m->mName = material.name;

    for (auto const& p : material.values) {
      if (p.first == "baseColorTexture")
        m->mBaseColorTexture = textures[p.second.TextureIndex()];
      else if (p.first == "metallicRoughnessTexture")
        m->mMetallicRoughnessTexture = textures[p.second.TextureIndex()];
      else if (p.first == "metallicFactor")
        m->mPushConstants.mMetallicFactor = p.second.Factor();
      else if (p.first == "roughnessFactor")
        m->mPushConstants.mRoughnessFactor = p.second.Factor();
      else if (p.first == "baseColorFactor") {
        auto fac                           = p.second.ColorFactor();
        m->mPushConstants.mBaseColorFactor = glm::vec4(fac[0], fac[1], fac[2], fac[3]);
      }
    }

    for (auto const& p : material.additionalValues) {
      if (p.first == "normalTexture")
        m->mNormalTexture = textures[p.second.TextureIndex()];
      else if (p.first == "occlusionTexture")
        m->mOcclusionTexture = textures[p.second.TextureIndex()];
      else if (p.first == "emissiveTexture")
        m->mEmissiveTexture = textures[p.second.TextureIndex()];
      else if (p.first == "normalScale")
        m->mPushConstants.mNormalScale = p.second.Factor();
      else if (p.first == "alphaCutoff")
        m->mPushConstants.mAlphaCutoff = p.second.Factor();
      else if (p.first == "occlusionStrength")
        m->mPushConstants.mOcclusionStrength = p.second.Factor();
      else if (p.first == "emissiveFactor") {
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
      }
    }

    mMaterials.push_back(m);
  }

  // create nodes & primitives ---------------------------------------------------------------------
  {
    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex>   vertexBuffer;

    std::function<void(Node&, tinygltf::Node const&)> addNode = [&](Node&               parent,
                                                                  tinygltf::Node const& n) {
      Node node;
      node.mModelMatrix = parent.mModelMatrix;
      node.mName        = n.name;

      if (n.matrix.size() > 0) {
        node.mModelMatrix *= glm::make_mat4(n.matrix.data());
      } else {
        if (n.translation.size() > 0) {
          node.mModelMatrix =
            glm::translate(node.mModelMatrix, glm::make_vec3(n.translation.data()));
        }
        if (n.rotation.size() > 0) {
          glm::dquat quaternion(glm::make_quat(n.rotation.data()));
          node.mModelMatrix =
            glm::rotate(node.mModelMatrix, glm::angle(quaternion), glm::axis(quaternion));
        }
        if (n.scale.size() > 0) {
          node.mModelMatrix = glm::scale(node.mModelMatrix, glm::make_vec3(n.scale.data()));
        }
      }

      if (n.mesh >= 0) {
        auto const& mesh = model.meshes[n.mesh];

        for (auto const& p : mesh.primitives) {
          Primitive primitve;

          primitve.mMaterial = mMaterials[p.material];
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

            primitve.mMinPosition = glm::min(primitve.mMinPosition, vertex.mPosition);
            primitve.mMaxPosition = glm::max(primitve.mMaxPosition, vertex.mPosition);

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

          node.mMinPosition = glm::min(node.mMinPosition, primitve.mMinPosition);
          node.mMaxPosition = glm::max(node.mMaxPosition, primitve.mMaxPosition);
          node.mPrimitives.emplace_back(primitve);
        }
      }

      // add children
      for (int c : n.children) {
        addNode(node, model.nodes[c]);
      }

      parent.mChildren.emplace_back(node);
      parent.mMinPosition = glm::min(parent.mMinPosition, node.mMinPosition);
      parent.mMaxPosition = glm::max(parent.mMaxPosition, node.mMaxPosition);
    };

    // add all default scene nodes
    for (int n : model.scenes[model.defaultScene].nodes) {
      addNode(mRootNode, model.nodes[n]);
    }

    mVertexBuffer = mDevice->createVertexBuffer(vertexBuffer);
    mIndexBuffer  = mDevice->createIndexBuffer(indexBuffer);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<GltfModel::Node> const& GltfModel::getNodes() const { return mRootNode.mChildren; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::array<glm::vec3, 2> GltfModel::getAABB() const {
  return {mRootNode.mMinPosition, mRootNode.mMaxPosition};
}

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
