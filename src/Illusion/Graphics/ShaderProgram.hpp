////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADER_PROGRAM_HPP
#define ILLUSION_GRAPHICS_SHADER_PROGRAM_HPP

#include "DescriptorPool.hpp"

#include <map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderProgram {

 public:
  static ShaderProgramPtr createFromFiles(
    DevicePtr const& device, std::vector<std::string> const& files);

  static ShaderProgramPtr createFromGlsl(
    DevicePtr const& device, std::map<vk::ShaderStageFlagBits, std::string> const& sources);

  template <typename... Args>
  static ShaderProgramPtr create(Args&&... args) {
    return std::make_shared<ShaderProgram>(args...);
  };

  ShaderProgram(DevicePtr const& device, std::vector<ShaderModulePtr> const& modules);
  virtual ~ShaderProgram();

  std::vector<ShaderModulePtr> const& getModules() const;

  PipelineReflectionPtr const&                   getReflection() const;
  std::vector<DescriptorSetReflectionPtr> const& getDescriptorSetReflections() const;

 private:
  void createReflection();

  DevicePtr                    mDevice;
  std::vector<ShaderModulePtr> mModules;
  PipelineReflectionPtr        mReflection;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_PROGRAM_HPP
