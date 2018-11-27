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
  static std::shared_ptr<ShaderProgram> createFromGlslFiles(
    std::shared_ptr<Device> const&                                  device,
    std::unordered_map<vk::ShaderStageFlagBits, std::string> const& files);

  ShaderProgram(
    std::shared_ptr<Device> const&                    device,
    std::vector<std::shared_ptr<ShaderModule>> const& modules);
  virtual ~ShaderProgram();

  std::vector<std::shared_ptr<ShaderModule>> const& getModules() const;

  std::shared_ptr<PipelineReflection> const&                          getReflection() const;
  std::map<uint32_t, std::shared_ptr<DescriptorSetReflection>> const& getDescriptorSetReflections()
    const;

 private:
  void createReflection();

  std::shared_ptr<Device>                    mDevice;
  std::vector<std::shared_ptr<ShaderModule>> mModules;
  std::shared_ptr<PipelineReflection>        mReflection;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_PROGRAM_HPP
