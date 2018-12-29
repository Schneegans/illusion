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

#include "../Core/Flags.hpp"
#include "fwd.hpp"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <tiny_gltf.h>

namespace tinygltf {
class Model;
}

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class GltfModel {
 public:
  struct Material;
  struct Mesh;
  struct Node;
  struct Animation;
  struct Skin;

  typedef std::shared_ptr<Material>  MaterialPtr;
  typedef std::shared_ptr<Mesh>      MeshPtr;
  typedef std::shared_ptr<Node>      NodePtr;
  typedef std::shared_ptr<Animation> AnimationPtr;
  typedef std::shared_ptr<Skin>      SkinPtr;

  struct TextureChannelMapping {
    enum class Channel { eRed, eGreen, eBlue };

    TextureChannelMapping()
        : mOcclusion(Channel::eRed)
        , mRoughness(Channel::eGreen)
        , mMetallic(Channel::eBlue) {
    }

    TextureChannelMapping(Channel occlusion, Channel roughness, Channel metallic)
        : mOcclusion(occlusion)
        , mRoughness(roughness)
        , mMetallic(metallic) {
    }

    Channel mOcclusion;
    Channel mRoughness;
    Channel mMetallic;
  };

  struct Material {

    bool mDoubleSided     = false;
    bool mDoAlphaBlending = false;

    glm::vec4 mAlbedoFactor               = glm::vec4(1.f); // diffuse factor for SG-Workflow
    glm::vec3 mEmissiveFactor             = glm::vec3(0.f);
    bool      mSpecularGlossinessWorkflow = false;
    glm::vec3 mMetallicRoughnessFactor    = glm::vec3(1.f); // specular factor for SG-Workflow
    float     mNormalScale                = 1.f;
    float     mOcclusionStrength          = 1.f;
    float     mAlphaCutoff                = 0.5f;

    TexturePtr mAlbedoTexture;            // rgba: baseColor / diffuseColor
    TexturePtr mEmissiveTexture;          // rgb:  emissivity
    TexturePtr mMetallicRoughnessTexture; // g:    roughness, b: metallic / rgb specular glossiness
    TexturePtr mOcclusionTexture;         // r:    ambient occlusion
    TexturePtr mNormalTexture;            // rgb:  tangent space normal map

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

    enum class VertexAttributeBits : int {
      eNormals   = 1 << 0,
      eTexcoords = 1 << 1,
      eSkins     = 1 << 2
    };

    Core::Flags<VertexAttributeBits> mVertexAttributes;
    MaterialPtr                      mMaterial;
    vk::PrimitiveTopology            mTopology;
    vk::DeviceSize                   mIndexCount;
    vk::DeviceSize                   mIndexOffset;
    BoundingBox                      mBoundingBox;
  };

  struct Mesh {
    std::string            mName;
    BoundingBox            mBoundingBox;
    std::vector<Primitive> mPrimitives;
  };

  struct Node {
    std::string mName;

    glm::mat4 mGlobalTransform = glm::mat4(1.f);

    glm::mat4 mTransform   = glm::mat4(1.f);
    glm::vec3 mTranslation = glm::vec3(0.f);
    glm::quat mRotation    = glm::quat(1.f, 0.f, 0.f, 0.f);
    glm::vec3 mScale       = glm::vec3(1.f);

    glm::vec3 mRestTranslation = glm::vec3(0.f);
    glm::quat mRestRotation    = glm::quat(1.f, 0.f, 0.f, 0.f);
    glm::vec3 mRestScale       = glm::vec3(1.f);

    MeshPtr              mMesh;
    SkinPtr              mSkin;
    std::vector<NodePtr> mChildren;

    void update(glm::mat4 parentTransform) {
      mGlobalTransform = parentTransform * getLocalTransform();

      for (auto const& c : mChildren) {
        c->update(mGlobalTransform);
      }
    }

    glm::mat4 getLocalTransform() const {
      glm::mat4 transform(mTransform);
      transform = glm::translate(transform, mTranslation);
      transform = glm::rotate(transform, glm::angle(mRotation), glm::axis(mRotation));
      transform = glm::scale(transform, mScale);
      return transform;
    };

    BoundingBox getBoundingBox() const {
      BoundingBox bbox;
      addMeshesToBoundingBox(bbox, glm::mat4(1.f));
      return bbox;
    }

    void addMeshesToBoundingBox(BoundingBox& bbox, glm::mat4 const& parentTransform) const {
      glm::mat4 transform = parentTransform * getLocalTransform();
      if (mMesh) {
        bbox.add(mMesh->mBoundingBox.getTransformed(transform));
      }
      for (auto const& c : mChildren) {
        c->addMeshesToBoundingBox(bbox, transform);
      }
    }
  };

  struct Animation {
    struct Channel {
      enum class Type { eTranslation, eRotation, eScale };
      Type     mType;
      NodePtr  mNode;
      uint32_t mSamplerIndex;
    };

    struct Sampler {
      enum class Type { eLinear, eStep, eCubicSpline };
      Type                   mType;
      std::vector<float>     mKeyFrames;
      std::vector<glm::vec4> mValues;
    };

    std::string          mName;
    std::vector<Sampler> mSamplers;
    std::vector<Channel> mChannels;
    float                mStart = std::numeric_limits<float>::max();
    float                mEnd   = std::numeric_limits<float>::min();
  };

  struct Skin {
    std::string            mName;
    std::vector<glm::mat4> mInverseBindMatrices;
    std::vector<NodePtr>   mJoints;

    std::vector<glm::mat4> getJointMatrices(glm::mat4 const& meshTransform) const {
      std::vector<glm::mat4> jointMatrices(mJoints.size());

      glm::mat4 inverseMeshTransform = glm::inverse(meshTransform);

      for (size_t i(0); i < mJoints.size(); i++) {
        jointMatrices[i] =
            inverseMeshTransform * mJoints[i]->mGlobalTransform * mInverseBindMatrices[i];
      }

      return jointMatrices;
    }
  };

  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static GltfModelPtr create(Args&&... args) {
    return std::make_shared<GltfModel>(args...);
  };

  enum class OptionFlagBits : int {
    eNone       = 0,
    eAnimations = 1 << 0,
    eSkins      = 1 << 1,
    eTextures   = 1 << 2,
    eAll        = eAnimations | eSkins | eTextures
  };

  typedef Core::Flags<OptionFlagBits> OptionFlags;

  GltfModel(DevicePtr const& device, std::string const& file,
      OptionFlags                  options         = OptionFlagBits::eAll,
      TextureChannelMapping const& textureChannels = TextureChannelMapping());

  void setAnimationTime(uint32_t animationIndex, float time);

  Node const&            getRoot() const;
  BackedBufferPtr const& getIndexBuffer() const;
  BackedBufferPtr const& getVertexBuffer() const;

  std::vector<TexturePtr> const&   getTextures() const;
  std::vector<MaterialPtr> const&  getMaterials() const;
  std::vector<MeshPtr> const&      getMeshes() const;
  std::vector<NodePtr> const&      getNodes() const;
  std::vector<AnimationPtr> const& getAnimations() const;

  void update() {
    mRootNode.update(glm::mat4(1.f));
  }

  void printInfo() const;

  static std::vector<vk::VertexInputBindingDescription>   getVertexInputBindings();
  static std::vector<vk::VertexInputAttributeDescription> getVertexInputAttributes();

 private:
  DevicePtr       mDevice;
  Node            mRootNode;
  BackedBufferPtr mIndexBuffer;
  BackedBufferPtr mVertexBuffer;

  std::vector<TexturePtr>   mTextures;
  std::vector<MaterialPtr>  mMaterials;
  std::vector<MeshPtr>      mMeshes;
  std::vector<NodePtr>      mNodes;
  std::vector<AnimationPtr> mAnimations;
  std::vector<SkinPtr>      mSkins;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GLTF_MODEL_HPP
