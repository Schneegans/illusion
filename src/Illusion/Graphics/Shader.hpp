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
  // Convenience method to create a shader and add a ShaderModule::GlslFile module for each given
  // file name. The shader stage is automatically determined by the file name ending.
  // *.frag: Fragment
  // *.vert: Vertex
  // *.geom: Geometry
  // *.comp: Compute
  // *.tesc: TessellationControl
  // *.tese: TessellationEvaluation
  static ShaderPtr createFromGlslFiles(DevicePtr const& device,
      std::vector<std::string> const& fileNames, bool reloadOnChanges = true,
      std::set<std::string> dynamicBuffers = {});

  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static ShaderPtr create(Args&&... args) {
    return std::make_shared<Shader>(args...);
  };

  // Creates an "empty" shader program with no modules attached to. Use the method addModule() to
  // add modules for each required shader stage.
  Shader(DevicePtr const& device);
  virtual ~Shader();

  // Adds a shader module to this Shader. No Vulkan resources are allocated by this call, only an
  // internal dirty flag is set. The creation of the ShaderModule and the shader reflection happens
  // lazily when one of the methods below gets called.
  // If this method is called multiple times for the same stage, the previous data will be
  // overridden.
  void addModule(vk::ShaderStageFlagBits stage, ShaderModule::Source const& source,
      std::set<std::string> const& dynamicBuffers = {});

  // Returns a vector of ShaderModules. These are allocated lazily by this call and can be queried
  // for the actual Vulkan handle.
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
