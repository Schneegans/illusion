////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_GLSL_SHADER_HPP
#define ILLUSION_GRAPHICS_GLSL_SHADER_HPP

#include "Shader.hpp"

#include <map>
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class GlslShader : public Shader {

 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  static GlslShaderPtr create(DevicePtr const& device, std::vector<std::string> const& fileNames,
      std::set<std::string> const& dynamicBuffers = {}) {
    return std::make_shared<GlslShader>(device, fileNames, dynamicBuffers);
  }

  GlslShader(DevicePtr const& device, std::vector<std::string> const& fileNames,
      std::set<std::string> const& dynamicBuffers = {});

  virtual ~GlslShader();

  virtual std::vector<ShaderModulePtr> const&            getModules() override;
  virtual PipelineReflectionPtr const&                   getReflection() override;
  virtual std::vector<DescriptorSetReflectionPtr> const& getDescriptorSetReflections() override;

 protected:
  void loadFromFiles();
  void reload();

  std::set<std::string>                mDynamicBuffers;
  std::vector<std::string>             mFileNames;
  std::vector<Core::File<std::string>> mAllSourceFiles;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GLSL_SHADER_HPP
