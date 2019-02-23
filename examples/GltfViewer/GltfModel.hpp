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
// While Illusion has a class for loading glTF models, we need to provide the rendering code on   //
// the application side. This is what this class does.                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////

class GltfModel {
 public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // This struct is used as a push constant block in GltfShader.vert and GltfShader.frag. It      //
  // requires 124 bit which is pretty close to the guaranteed minimum of 128 bit.                 //
  //////////////////////////////////////////////////////////////////////////////////////////////////

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

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // In this example we support a maximum number of 256 joints per glTF model. This requires      //
  // exactly the guaranteed minimum uniform buffer size of 16 kiB. If we would need more, we      //
  // should use storage buffers instead.                                                          //
  //////////////////////////////////////////////////////////////////////////////////////////////////

  struct SkinUniforms {
    glm::mat4 mJointMatrices[256];
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////

  GltfModel(std::string const& name, Illusion::Graphics::DeviceConstPtr const& device,
      std::string const& fileName, Illusion::Graphics::Gltf::LoadOptions const& options,
      Illusion::Graphics::FrameResourceIndexPtr const& frameIndex);

  // If animations were loaded,t this will update the animation state of all nodes of the model. If
  // skins were loaded, those will get updated as well.
  void update(double time, int32_t animation);

  // This will first draw all nodes with mDoAlphaBlending == false and then all nodes with
  // mDoAlphaBlending == true in order to get correct composition order. In a more complete engine
  // this should be done in different passes.
  void draw(Illusion::Graphics::CommandBufferPtr const& cmd, glm::mat4 const& viewMatrix);

 private:
  void drawNodes(Illusion::Graphics::CommandBufferPtr const&              cmd,
      std::vector<std::shared_ptr<Illusion::Graphics::Gltf::Node>> const& nodes,
      glm::mat4 const& viewMatrix, bool doAlphaBlending);

  Illusion::Graphics::DeviceConstPtr mDevice;
  Illusion::Graphics::Gltf::ModelPtr mModel;
  Illusion::Graphics::ShaderPtr      mShader;

  // For each skin a uniform buffer is created as a FrameResource.
  Illusion::Graphics::FrameResource<
      std::unordered_map<Illusion::Graphics::Gltf::SkinPtr, Illusion::Graphics::CoherentBufferPtr>>
      mSkinBuffers;

  // As we need to bind something, we will bind this empty uniform buffer if the currently drawn
  // node has no associated skin.
  Illusion::Graphics::CoherentBufferPtr mEmptySkinBuffer;

  glm::mat4 mModelMatrix{};
};

#endif // ILLUSION_EXAMPLES_GLTF_VIEWER_GLTF_MODEL_HPP
