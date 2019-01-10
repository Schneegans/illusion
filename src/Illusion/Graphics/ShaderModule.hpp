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
#include "ShaderSource.hpp"
#include "fwd.hpp"

#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderModule {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static ShaderModulePtr create(Args&&... args) {
    return std::make_shared<ShaderModule>(args...);
  };

  ShaderModule(DevicePtr const& device, ShaderSourcePtr const& source,
      vk::ShaderStageFlagBits stage, std::set<std::string> const& dynamicBuffers = {});

  virtual ~ShaderModule();

  bool requiresReload() const;
  void resetReloadingRequired();
  void reload();

  vk::ShaderModulePtr                  getHandle() const;
  vk::ShaderStageFlagBits              getStage() const;
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
