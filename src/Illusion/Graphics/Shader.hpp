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
#include "ShaderModule.hpp"

#include <map>
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Shader {

 public:
  static ShaderPtr createFromGlslFiles(DevicePtr const& device,
      std::vector<std::string> const& fileNames, bool reloadOnChanges = true,
      std::set<std::string> dynamicBuffers = {});

  // Syntactic sugar to create a std::shared_ptr for this class
  static ShaderPtr create(DevicePtr const& device) {
    return std::make_shared<Shader>(device);
  }

  Shader() = default;
  Shader(DevicePtr const& device);

  virtual ~Shader();

  void addModule(vk::ShaderStageFlagBits stage, ShaderModule::Source const& source,
      std::set<std::string> const& dynamicBuffers = {});

  std::vector<ShaderModulePtr> const&            getModules();
  PipelineReflectionPtr const&                   getReflection();
  std::vector<DescriptorSetReflectionPtr> const& getDescriptorSetReflections();

 private:
  void reload();

  DevicePtr                    mDevice;
  std::vector<ShaderModulePtr> mModules;
  PipelineReflectionPtr        mReflection;

  bool                                                               mDirty = false;
  std::unordered_map<vk::ShaderStageFlagBits, ShaderModule::Source>  mSources;
  std::unordered_map<vk::ShaderStageFlagBits, std::set<std::string>> mDynamicBuffers;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_HPP
