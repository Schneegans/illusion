////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADER_HPP
#define ILLUSION_GRAPHICS_SHADER_HPP

#include "../Core/File.hpp"
#include "DescriptorPool.hpp"

#include <map>
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Shader {

 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  static ShaderPtr create(DevicePtr const& device, std::vector<ShaderModulePtr> const& modules) {
    return std::make_shared<Shader>(device, modules);
  }

  Shader() = default;
  Shader(DevicePtr const& device, std::vector<ShaderModulePtr> const& modules);

  virtual ~Shader();

  virtual std::vector<ShaderModulePtr> const&            getModules();
  virtual PipelineReflectionPtr const&                   getReflection();
  virtual std::vector<DescriptorSetReflectionPtr> const& getDescriptorSetReflections();

 protected:
  void createReflection();

  DevicePtr                    mDevice;
  std::vector<ShaderModulePtr> mModules;
  PipelineReflectionPtr        mReflection;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_HPP
