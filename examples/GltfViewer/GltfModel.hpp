////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_EXAMPLES_GLTF_VIEWER_GLTF_MODEL_HPP
#define ILLUSION_EXAMPLES_GLTF_VIEWER_GLTF_MODEL_HPP

#include <Illusion/Graphics/FrameResource.hpp>
#include <Illusion/Graphics/GltfModel.hpp>
#include <Illusion/Graphics/fwd.hpp>

#include <glm/glm.hpp>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class GltfModel {
 public:
  ////////////////////////////////////////////////////////////////////////////////////////////////
  // This struct is used as a push constant block in GltfShader.vert and GltfShader.frag. It     /
  // requires 124 bit which is pretty close to the guaranteed minimum of 128 bit.                /
  ////////////////////////////////////////////////////////////////////////////////////////////////

  struct PushConstants {

    // The current model matrix of the glTF node. ViewMatrix and ProjectionMatrix are set via a
    // uniform buffer object.
    glm::mat4 mModelMatrix;

    // The following members are all glTF material properties which are not textures. Its pretty
    // cool that they fit into push constants because we can safe a lot of uniform buffer memory
    // this way.
    glm::vec4 mAlbedoFactor;
    glm::vec3 mEmissiveFactor;
    bool      mSpecularGlossinessWorkflow;
    glm::vec3 mMetallicRoughnessFactor;
    float     mNormalScale;
    float     mOcclusionStrength;
    float     mAlphaCutoff;

    // This integer contains some bits specifying which vertex attributes are actually set. See
    // GltfModel::Primitive::VertexAttributeBits for a list of bits.
    int32_t mVertexAttributes;
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // In this example we support a maximum number of 256 joints per glTF model.
  /////////////////////////////////////////////////////////////////////////////////////////////////

  struct SkinUniforms {
    glm::mat4 mJointMatrices[256];
  };

  GltfModel(std::string const& name, Illusion::Graphics::DevicePtr const& device,
      std::string const& fileName, Illusion::Graphics::Gltf::LoadOptions const& options,
      Illusion::Graphics::FrameResourceIndexPtr const& frameIndex);

  void update(double time, int32_t animation);

  void draw(Illusion::Graphics::CommandBufferPtr const& cmd, glm::mat4 const& viewMatrix);

 private:
  void drawNodes(Illusion::Graphics::CommandBufferPtr const&              cmd,
      std::vector<std::shared_ptr<Illusion::Graphics::Gltf::Node>> const& nodes,
      glm::mat4 const& viewMatrix, bool doAlphaBlending);

  Illusion::Graphics::DevicePtr      mDevice;
  Illusion::Graphics::Gltf::ModelPtr mModel;
  Illusion::Graphics::ShaderPtr      mShader;

  Illusion::Graphics::FrameResource<
      std::unordered_map<Illusion::Graphics::Gltf::SkinPtr, Illusion::Graphics::CoherentBufferPtr>>
      mSkinBuffers;

  Illusion::Graphics::CoherentBufferPtr mEmptySkinBuffer;

  glm::mat4 mModelMatrix;
};

#endif // ILLUSION_EXAMPLES_GLTF_VIEWER_GLTF_MODEL_HPP
