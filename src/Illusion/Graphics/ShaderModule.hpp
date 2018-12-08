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

#include "PipelineResource.hpp"
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderModule {
 public:
  static std::vector<uint32_t> compileGlsl(std::string const& glsl, vk::ShaderStageFlagBits stage);

  template <typename... Args>
  static EnginePtr create(Args&&... args) {
    return std::make_shared<Engine>(args...);
  };

  ShaderModule(DevicePtr const& device, std::string const& glsl, vk::ShaderStageFlagBits stage);

  ShaderModule(
    DevicePtr const& device, std::vector<uint32_t>&& spirv, vk::ShaderStageFlagBits stage);

  virtual ~ShaderModule();

  vk::ShaderStageFlagBits              getStage() const;
  vk::ShaderModulePtr                  getModule() const;
  std::vector<PipelineResource> const& getResources() const;

 private:
  void createReflection();

  std::vector<uint32_t>         mSpirv;
  vk::ShaderStageFlagBits       mStage;
  vk::ShaderModulePtr           mModule;
  std::vector<PipelineResource> mResources;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADERMODULE_HPP
