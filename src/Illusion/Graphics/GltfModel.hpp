////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_GLTF_MODEL_HPP
#define ILLUSION_GRAPHICS_GLTF_MODEL_HPP

#include "../Core/Flags.hpp"
#include "../Core/NamedObject.hpp"
#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Illusion::Graphics::Gltf {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This namespace contains the Gltf::Model class and several structs which are members of the     //
// Model. As a user, you will instantiate a Model providing a file name of a glTF model (.gltf or //
// .glb). All other structs will be instantiated as part of the loading process.                  //
// For now, all members of the structs are public. This should change in future as.               //
////////////////////////////////////////////////////////////////////////////////////////////////////

// A bitwise combination of these flags can be passed to the constructor of the Model.
enum class LoadOptionBits : int32_t {
  eNone       = 0,
  eAnimations = 1 << 0,
  eSkins      = 1 << 1,
  eTextures   = 1 << 2,
  eAll        = eAnimations | eSkins | eTextures
};

typedef Core::Flags<LoadOptionBits> LoadOptions;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Given a filename of a .gltf or .glb file, the Gltf::Model will load all Nodes, Materials,      //
// Textures, Meshes, Primitives, Animations and Skins from the file. All vertex data of all       //
// primitives is stored in one huge vertex buffer and one index buffer object. The primitives     //
// only store information on the data offset in those buffers. While this leads to some wasting   //
// of memory (not all primitives will have normals, texture coordinates and joint information),   //
// this makes rendering of Models much cheaper since no pipeline need to be re-bound.             //
//                                                                                                //
// For now, multiple scenes, sparse accessors and morph targets are not supported.                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Model : public Core::StaticCreate<Model>, public Core::NamedObject {
 public:
  // Creates a new Gltf::Model. The fileName should either be a *.gltf or a *.glb file. With the
  // options parameter you can prevent loading of some components such as textures. It is a good
  // idea to give the object a descriptive name. There are several reasons why this could throw a
  // std::runtime_error. It is a good idea to catch those cases and report the error message to the
  // user.
  Model(std::string const& name, DevicePtr device, std::string const& fileName,
      LoadOptions const& options = LoadOptionBits::eAll);

  // Updates all transformations of all Nodes according to the given animation and time. The time is
  // automatically clamped to the start and end time of the animation and is usually provided in
  // seconds. This will throw a std::runtime_error when animationIndex is not supported.
  void setAnimationTime(uint32_t animationIndex, float time);

  // Gets the root node of the default scene. This usually does not exist in the glTF format but is
  // created here anyways. It is quite useful for getting the global bounding box, for example. The
  // children of this Node are the actual root nodes of the glTF file.
  NodePtr const& getRoot() const;

  // Returns the index buffer for all primitives of this Model.
  BackedBufferPtr const& getIndexBuffer() const;

  // Returns the vertex buffer for all primitives of this Model.
  BackedBufferPtr const& getVertexBuffer() const;

  // The Nodes store pointers to their Materials / Meshes / ... but it may be useful to access all
  // of them in one std::vector. Especially the Animations should be accessed via this API.
  std::vector<TexturePtr> const&   getTextures() const;
  std::vector<MaterialPtr> const&  getMaterials() const;
  std::vector<MeshPtr> const&      getMeshes() const;
  std::vector<NodePtr> const&      getNodes() const;
  std::vector<AnimationPtr> const& getAnimations() const;
  std::vector<SkinPtr> const&      getSkins() const;

  // For debugging purposes.
  void printInfo() const;

  // Since all vertices are stored in one vertex buffer object, these are the same for all Models.
  static std::vector<vk::VertexInputBindingDescription>   getVertexInputBindings();
  static std::vector<vk::VertexInputAttributeDescription> getVertexInputAttributes();

 private:
  DevicePtr       mDevice;
  NodePtr         mRootNode;
  BackedBufferPtr mIndexBuffer;
  BackedBufferPtr mVertexBuffer;

  std::vector<TexturePtr>   mTextures;
  std::vector<MaterialPtr>  mMaterials;
  std::vector<MeshPtr>      mMeshes;
  std::vector<NodePtr>      mNodes;
  std::vector<AnimationPtr> mAnimations;
  std::vector<SkinPtr>      mSkins;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Material can be either used for the metallic-roughness workflow or for the                 //
// specular-glossiness workflow. In the latter case, some of the members are intrepreted in a     //
// different way. See the inline comments for details.                                            //
// When drawing the model, it is possible to upload the material data via push constants.         //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Material {
  std::string mName;

  bool mDoubleSided                = false;
  bool mDoAlphaBlending            = false;
  bool mSpecularGlossinessWorkflow = false;

  glm::vec4 mAlbedoFactor            = glm::vec4(1.f); // diffuse factor for SG-Workflow
  glm::vec3 mEmissiveFactor          = glm::vec3(0.f);
  glm::vec3 mMetallicRoughnessFactor = glm::vec3(1.f); // specular factor for SG-Workflow
  float     mNormalScale             = 1.f;
  float     mOcclusionStrength       = 1.f;
  float     mAlphaCutoff             = 0.5f;

  TexturePtr mAlbedoTexture;            // rgba: baseColor / diffuseColor
  TexturePtr mEmissiveTexture;          // rgb:  emissivity
  TexturePtr mMetallicRoughnessTexture; // g:    roughness, b: metallic / rgb specular glossiness
  TexturePtr mOcclusionTexture;         // r:    ambient occlusion
  TexturePtr mNormalTexture;            // rgb:  tangent space normal map
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// A simple axis aligned bounding box. Once we have more sophisticated math types, this might     //
// move there.                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct BoundingBox {
  glm::vec3 mMax = glm::vec3(std::numeric_limits<float>::lowest());
  glm::vec3 mMin = glm::vec3(std::numeric_limits<float>::max());

  // Returns a new axis aligned bounding box which contains this bounding box when transformed by
  // the given matrix.
  BoundingBox getTransformed(glm::mat4 const& transform);

  // Returns true when the mMin and mMax members have not been changed.
  bool isEmpty() const;

  // Increases the size of the box to contain the given point.
  void add(glm::vec3 const& point);

  // Increases the size of the box to contain the given box.
  void add(BoundingBox const& box);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// For now, all Gltf::Models share the same vertex layout. This simplifies drawing but wastes     //
// some memory. We should at least consider optimizing for the case were there are no animations  //
// at all. This would half the size of our vertex buffer object.                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Vertex {
  glm::vec3 mPosition  = glm::vec3(0.f);
  glm::vec3 mNormal    = glm::vec3(0.f);
  glm::vec2 mTexcoords = glm::vec2(0.f);
  glm::vec4 mJoint0    = glm::vec4(0.f);
  glm::vec4 mWeight0   = glm::vec4(0.f);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// A Primitive stores the offset into the Model-global index buffer object. Additionally it       //
// stores whether its vertices have normals, texture coordinates or joints and weights. As all    //
// vertices share the same layout, the Shader has to ignore those values if they are not actually //
// set. So it's a good idea to set the mVertexAttributes member as push constant at draw time.    //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Primitive {
  enum class VertexAttributeBits : int32_t {
    eNormals   = 1 << 0,
    eTexcoords = 1 << 1,
    eSkins     = 1 << 2
  };

  Core::Flags<VertexAttributeBits> mVertexAttributes;
  MaterialPtr                      mMaterial;
  vk::PrimitiveTopology            mTopology;
  vk::DeviceSize                   mIndexCount;
  uint32_t                         mIndexOffset;
  BoundingBox                      mBoundingBox;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// A Mesh contains a set of Primitives as well as a BoundingBox containing all Primitives.        //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Mesh {
  std::string            mName;
  BoundingBox            mBoundingBox;
  std::vector<Primitive> mPrimitives;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Gltf::Model contains a hierachy of nodes, each node may have a Mesh, a Skin and multiple   //
// child Nodes. Its transformation can be updated by Animations.                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Node {
  std::string          mName;
  MeshPtr              mMesh;
  SkinPtr              mSkin;
  std::vector<NodePtr> mChildren;

  // This is set by the update() method.
  glm::mat4 mGlobalTransform = glm::mat4(1.f);

  // These are affected by animations.
  glm::mat4 mTransform   = glm::mat4(1.f);
  glm::vec3 mTranslation = glm::vec3(0.f);
  glm::quat mRotation    = glm::quat(1.f, 0.f, 0.f, 0.f);
  glm::vec3 mScale       = glm::vec3(1.f);

  // These contain the original transformation as given by the glTF file.
  glm::vec3 mRestTranslation = glm::vec3(0.f);
  glm::quat mRestRotation    = glm::quat(1.f, 0.f, 0.f, 0.f);
  glm::vec3 mRestScale       = glm::vec3(1.f);

  // This is called recursively by the Gltf::Model's constructor and setAnimationTime() and updates
  // the mGlobalTransform member.
  void update(glm::mat4 parentTransform);

  // Combines the mTransform, mTranslation, mRotation and mScale members to one matrix.
  glm::mat4 getLocalTransform() const;

  // Calls addMeshesToBoundingBox() recursively on all children in order to compute the Node's
  // bounding box based on the current animation state.
  BoundingBox getBoundingBox() const;
  void        addMeshesToBoundingBox(BoundingBox& bbox, glm::mat4 const& parentTransform) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Animations define how Nodes move. Each Gltf::Model may have multiple animations, however only  //
// one can be used at a time.                                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Animation {

  // The Channel describes which Node to move.
  struct Channel {
    enum class Type { eTranslation, eRotation, eScale };
    Type     mType;
    NodePtr  mNode;
    uint32_t mSamplerIndex;
  };

  // The Sampler describes how to move the Node.
  struct Sampler {
    enum class Type { eLinear, eStep, eCubicSpline };
    Type                   mType;
    std::vector<float>     mKeyFrames;
    std::vector<glm::vec4> mValues;
  };

  std::string          mName;
  std::vector<Sampler> mSamplers;
  std::vector<Channel> mChannels;

  // In seconds.
  float mStart = std::numeric_limits<float>::max();
  float mEnd   = std::numeric_limits<float>::min();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// A Skin can deform the Mesh of a Node with a virtual skeleton.                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Skin {
  std::string            mName;
  std::vector<glm::mat4> mInverseBindMatrices;
  std::vector<NodePtr>   mJoints;
  NodePtr                mRoot;

  std::vector<glm::mat4> getJointMatrices() const;
};

} // namespace Illusion::Graphics::Gltf

#endif // ILLUSION_GRAPHICS_GLTF_MODEL_HPP
