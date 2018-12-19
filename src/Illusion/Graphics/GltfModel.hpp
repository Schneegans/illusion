////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_GLTF_MODEL_HPP
#define ILLUSION_GRAPHICS_GLTF_MODEL_HPP

#include "fwd.hpp"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <tiny_gltf.h>

namespace tinygltf {
class Model;
}

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class GltfModel {
 public:
  struct TextureChannelMapping {
    enum class Channel { eRed, eGreen, eBlue };

    TextureChannelMapping()
      : mOcclusion(Channel::eRed)
      , mRoughness(Channel::eGreen)
      , mMetallic(Channel::eBlue) {}

    TextureChannelMapping(Channel occlusion, Channel roughness, Channel metallic)
      : mOcclusion(occlusion)
      , mRoughness(roughness)
      , mMetallic(metallic) {}

    Channel mOcclusion;
    Channel mRoughness;
    Channel mMetallic;
  };

  struct Material {

    bool mDoubleSided     = false;
    bool mDoAlphaBlending = false;

    struct PushConstants {
      glm::vec4 mAlbedoFactor      = glm::vec4(1.f);
      glm::vec3 mEmissiveFactor    = glm::vec3(1.f);
      float     mMetallicFactor    = 1.f;
      float     mRoughnessFactor   = 1.f;
      float     mNormalScale       = 1.f;
      float     mOcclusionStrength = 1.f;
      float     mAlphaCutoff       = 0.5f;
    } mPushConstants;

    TexturePtr mAlbedoTexture;
    TexturePtr mMetallicRoughnessTexture;
    TexturePtr mNormalTexture;
    TexturePtr mOcclusionTexture;
    TexturePtr mEmissiveTexture;

    std::string mName;
  };

  struct BoundingBox {
    glm::vec3 mMax = glm::vec3(std::numeric_limits<float>::lowest());
    glm::vec3 mMin = glm::vec3(std::numeric_limits<float>::max());

    BoundingBox getTransformed(glm::mat4 const& transform) {
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

    bool isEmpty() const {
      return mMax == glm::vec3(std::numeric_limits<float>::lowest()) &&
             mMin == glm::vec3(std::numeric_limits<float>::max());
    }

    void add(glm::vec3 const& point) {
      mMin = glm::min(mMin, point);
      mMax = glm::max(mMax, point);
    }

    void add(BoundingBox const& box) {
      if (!box.isEmpty()) {
        add(box.mMin);
        add(box.mMax);
      }
    }
  };

  struct Vertex {
    glm::vec3 mPosition  = glm::vec3(0.f);
    glm::vec3 mNormal    = glm::vec3(0.f);
    glm::vec2 mTexcoords = glm::vec2(0.f);
    glm::vec4 mJoint0    = glm::vec4(0.f);
    glm::vec4 mWeight0   = glm::vec4(0.f);
  };

  struct Primitive {
    std::shared_ptr<Material> mMaterial;
    vk::PrimitiveTopology     mTopology;
    vk::DeviceSize            mIndexCount;
    vk::DeviceSize            mIndexOffset;
    BoundingBox               mBoundingBox;
  };

  struct Mesh {
    std::string            mName;
    BoundingBox            mBoundingBox;
    std::vector<Primitive> mPrimitives;
  };

  struct Node {
    std::string           mName;
    glm::dmat4            mModelMatrix = glm::dmat4(1);
    std::shared_ptr<Mesh> mMesh;
    std::vector<Node>     mChildren;

    BoundingBox getBoundingBox() const {
      BoundingBox bbox;
      addMeshesToBoundingBox(bbox, mModelMatrix);
      return bbox;
    }

    void addMeshesToBoundingBox(BoundingBox& bbox, glm::dmat4 parentTransform) const {
      auto transform = parentTransform * mModelMatrix;
      if (mMesh) {
        bbox.add(mMesh->mBoundingBox.getTransformed(mModelMatrix));
      }
      for (auto const& c : mChildren) {
        c.addMeshesToBoundingBox(bbox, transform);
      }
    }
  };

  template <typename... Args>
  static GltfModelPtr create(Args&&... args) {
    return std::make_shared<GltfModel>(args...);
  };

  GltfModel(DevicePtr const& device, std::string const& file,
    TextureChannelMapping const& textureChannels = TextureChannelMapping());

  std::vector<Node> const& getNodes() const;
  BoundingBox              getBoundingBox() const;

  BackedBufferPtr const& getIndexBuffer() const;
  BackedBufferPtr const& getVertexBuffer() const;

  void printInfo() const;

  static std::vector<vk::VertexInputBindingDescription>   getVertexInputBindings();
  static std::vector<vk::VertexInputAttributeDescription> getVertexInputAttributes();

 private:
  DevicePtr       mDevice;
  Node            mRootNode;
  BackedBufferPtr mIndexBuffer;
  BackedBufferPtr mVertexBuffer;

  std::vector<TexturePtr>                mTextures;
  std::vector<std::shared_ptr<Material>> mMaterials;
  std::vector<std::shared_ptr<Mesh>>     mMeshes;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GLTF_MODEL_HPP
