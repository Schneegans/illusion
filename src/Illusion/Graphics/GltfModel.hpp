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
  struct Material {

    enum class AlphaMode { eOpaque, eBlend, eMask };

    glm::vec4 mBaseColorFactor   = glm::vec4(1.f);
    glm::vec3 mEmissiveFactor    = glm::vec3(1.f);
    float     mMetallicFactor    = 1.f;
    float     mRoughnessFactor   = 1.f;
    float     mNormalScale       = 1.f;
    float     mOcclusionStrength = 1.f;
    float     mAlphaCutoff       = 0.f;
    AlphaMode mAlphaMode         = AlphaMode::eOpaque;

    TexturePtr mBaseColorTexture;
    TexturePtr mMetallicRoughnessTexture;
    TexturePtr mNormalTexture;
    TexturePtr mOcclusionTexture;
    TexturePtr mEmissiveTexture;

    std::string mName;
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
    glm::vec3                 mMaxPosition = glm::vec3(std::numeric_limits<float>::lowest());
    glm::vec3                 mMinPosition = glm::vec3(std::numeric_limits<float>::max());
  };

  struct Node {
    glm::dmat4             mModelMatrix = glm::dmat4(1);
    std::vector<Primitive> mPrimitives;
    std::vector<Node>      mChildren;
    glm::vec3              mMaxPosition = glm::vec3(std::numeric_limits<float>::lowest());
    glm::vec3              mMinPosition = glm::vec3(std::numeric_limits<float>::max());
    std::string            mName;
  };

  template <typename... Args>
  static GltfModelPtr create(Args&&... args) {
    return std::make_shared<GltfModel>(args...);
  };

  GltfModel(DevicePtr const& device, std::string const& file);

  std::vector<Node> const& getNodes() const;
  std::array<glm::vec3, 2> getAABB() const;

  BackedBufferPtr const& getIndexBuffer() const;
  BackedBufferPtr const& getVertexBuffer() const;

  static std::vector<vk::VertexInputBindingDescription>   getVertexInputBindings();
  static std::vector<vk::VertexInputAttributeDescription> getVertexInputAttributes();

 private:
  DevicePtr mDevice;

  Node                                   mRootNode;
  std::vector<std::shared_ptr<Material>> mMaterials;
  BackedBufferPtr                        mIndexBuffer;
  BackedBufferPtr                        mVertexBuffer;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GLTF_MODEL_HPP
