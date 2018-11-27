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

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

#include <glm/glm.hpp>
#include <tiny_gltf.h>

namespace tinygltf {
class Model;
}

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class GltfModel {
 public:
  struct PBRMaterialUniforms {

    // reflection information
    static vk::ShaderStageFlags getActiveStages() { return vk::ShaderStageFlagBits::eFragment; }
    static uint32_t             getBindingPoint() { return 0; }
    static uint32_t             getDescriptorSet() { return 1; }

    // struct members
    glm::vec3 color;
  };

  // -------------------------------------------------------------------------------- public methods
  GltfModel(std::shared_ptr<Illusion::Graphics::Device> const& device, std::string const& file);

  void predraw(vk::CommandBuffer const& cmd);

  void draw(
    vk::CommandBuffer const&           cmd,
    std::shared_ptr<RenderPass> const& renderPass,
    uint32_t                           subPass,
    glm::dmat4                         modelMatrix);

  void printInfo() const;

 private:
  // ------------------------------------------------------------------------------- private methods
  static vk::Filter             convertFilter(int value);
  static vk::SamplerMipmapMode  convertSamplerMipmapMode(int value);
  static vk::SamplerAddressMode convertSamplerAddressMode(int value);
  static vk::Format             convertFormat(int type, int componentType);
  static vk::PrimitiveTopology  convertPrimitiveTopology(int value);

  void                                              loadData();
  std::shared_ptr<Illusion::Graphics::Texture>      createTexture(int index) const;
  std::shared_ptr<Illusion::Graphics::BackedBuffer> createBuffer(int index);
  int getTextureIndex(int materialIndex, std::string const& name);

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Illusion::Graphics::Device> mDevice;
  std::string                                 mFile;
  tinygltf::Model                             mGLTF;

  std::vector<std::shared_ptr<Illusion::Graphics::Texture>>      mTextures;
  std::vector<std::shared_ptr<Illusion::Graphics::BackedBuffer>> mBuffers;

  std::vector<vk::DescriptorSet>   mDescriptorSets;
  std::vector<PBRMaterialUniforms> mUniformBuffers;
};

// -------------------------------------------------------------------------------------------------

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GLTF_MODEL_HPP
