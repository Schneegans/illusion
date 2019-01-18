////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
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
// The Shader class stores multiple ShaderModules. Depending on the added ShaderModules, it can   //
// be either a graphics or a compute shader. After all ShaderModules have been added, you can use //
// getReflection() to generate a matching vk::PipelineLayout.                                     //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Shader : public Core::StaticCreate<Shader> {

 public:
  // Convenience method to create a shader and add a GlslFile ShaderSource for each given file name.
  // The shader stage is automatically determined by the file name ending (glsl / hlsl).
  // .vert / .vs: Vertex Shader
  // .frag / .ps: Fragment Shader
  // .geom / .gs: Geometry Shader
  // .tesc / .hs: Tessellation Control Shader / Hull Shader
  // .tese / .ds: Tessellation Evaluation Shader / Domain Shader
  // .comp / .cs: Compute Shader
  static ShaderPtr createFromFiles(DevicePtr const& device,
      std::vector<std::string> const& fileNames, std::set<std::string> dynamicBuffers = {},
      bool reloadOnChanges = true);

  // Creates an "empty" shader program with no modules attached to. Use the method addModule() to
  // add modules for each required shader stage.
  Shader(DevicePtr const& device);
  virtual ~Shader();

  // Adds a shader module to this Shader. No Vulkan resources are allocated by this call, only an
  // internal dirty flag is set. The creation of the ShaderModule and the shader reflection happens
  // lazily when one of the methods below gets called.
  // The source can be one of the sources defined in ShaderSource.hpp.
  // If there are any uniform and storage buffers defined in your shader source which should be
  // dynamic in the reflection, you should provide their names in the dynamicBuffers parameter.
  // If this method is called multiple times for the same stage, the previous data will be
  // overridden.
  void addModule(vk::ShaderStageFlagBits stage, ShaderSourcePtr const& source,
      std::set<std::string> const& dynamicBuffers = {});

  // Returns a vector of ShaderModules. These are allocated lazily by this call and can be queried
  // for the actual Vulkan handle.
  std::vector<ShaderModulePtr> const& getModules();

  // The PipelineReflection can be used to query information on all resources of the contained
  // modules. It is primarily used to generate a corresponding vk::PipelineLayout.
  PipelineReflectionPtr const& getReflection();

  // This is just a convenience getter for the same method on the PipelineReflection. The
  // DescriptorSetReflection can be used to create a corresponding vk::DescriptorSetLayout.
  std::vector<DescriptorSetReflectionPtr> const& getDescriptorSetReflections();

 private:
  void reload();

  DevicePtr                    mDevice;
  std::vector<ShaderModulePtr> mModules;
  PipelineReflectionPtr        mReflection;

  bool                                                               mDirty = false;
  std::unordered_map<vk::ShaderStageFlagBits, ShaderSourcePtr>       mSources;
  std::unordered_map<vk::ShaderStageFlagBits, std::set<std::string>> mDynamicBuffers;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_HPP
