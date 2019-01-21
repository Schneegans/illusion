////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADERMODULE_HPP
#define ILLUSION_GRAPHICS_SHADERMODULE_HPP

#include "../Core/NamedObject.hpp"
#include "PipelineResource.hpp"
#include "ShaderSource.hpp"
#include "fwd.hpp"

#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The ShaderModule represents one stage of a Shader. This class is instantiated by the Shader    //
// class. You can use getModules() of the Shader class to get access to the modules allocated for //
// a Shader object. Internally, this class performs the actual shader reflection using            //
// SpirV-Cross. You can use getResources() to access the extracted resources, however the         //
// PipelineReflection of the Shader class provides some easier-to-use interfaces.                 //
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderModule : public Core::StaticCreate<ShaderModule>, public Core::NamedObject {
 public:
  // Creates a new ShaderModule. The source can be one of the sources defined in ShaderSource.hpp.
  // If there are any uniform and storage buffers defined in your shader source which should be
  // dynamic in the reflection, you should provide their names in the dynamicBuffers parameter.
  ShaderModule(std::string const& name, DevicePtr const& device, ShaderSourcePtr const& source,
      vk::ShaderStageFlagBits stage, std::set<std::string> const& dynamicBuffers = {});

  virtual ~ShaderModule();

  // This method is just forwarded to the contained ShaderSource. One reason for requiresReload() to
  // return true is when the ShaderSource is actually a file on disc and this file has been changed
  // since the last reloading of the ShaderMoule.
  bool requiresReload() const;

  // This method is also forwarded to the contained ShaderSource. For ShaderSources which are
  // actually files on disc, this will prevent requiresReload() to return true until the file is
  // changed once more.
  void resetReloadingRequired();

  // After this call, if no error occurs, the getHandle() method will return a new vk::ShaderModule.
  void reload();

  // Returns the internal vk::ShaderModule. Storing this might be a bad idea since a reload may
  // happen when shader files are changed on disc.
  vk::ShaderModulePtr getHandle() const;

  // Get the shader stage this module was constructed for.
  vk::ShaderStageFlagBits getStage() const;

  // Gets reflection information.
  std::vector<PipelineResource> const& getResources() const;

 private:
  void createReflection(std::vector<uint32_t> const& spirv);

  DevicePtr                     mDevice;
  vk::ShaderStageFlagBits       mStage;
  vk::ShaderModulePtr           mHandle;
  std::vector<PipelineResource> mResources;

  ShaderSourcePtr       mSource;
  std::set<std::string> mDynamicBuffers;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADERMODULE_HPP
