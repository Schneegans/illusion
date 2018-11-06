////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADERMODULE_HPP
#define ILLUSION_GRAPHICS_SHADERMODULE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "PipelineResource.hpp"
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class ShaderModule {
 public:
  static std::vector<uint32_t> compileGlsl(std::string const& glsl, vk::ShaderStageFlagBits stage);

  ShaderModule(std::string const& glsl, vk::ShaderStageFlagBits stage);
  ShaderModule(std::vector<uint32_t>&& spirv, vk::ShaderStageFlagBits stage);

  virtual ~ShaderModule();

  std::vector<uint32_t> const&         getSource() const;
  std::vector<PipelineResource> const& getReflection() const;

 private:
  void createReflection();

  std::vector<uint32_t>         mSpirv;
  vk::ShaderStageFlagBits       mStage;
  std::vector<PipelineResource> mReflection;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADERMODULE_HPP
