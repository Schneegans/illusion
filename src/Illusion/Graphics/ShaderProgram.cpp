////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "ShaderProgram.hpp"

#include "../Core/File.hpp"
#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "PipelineReflection.hpp"
#include "ShaderModule.hpp"
#include "Window.hpp"

#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ShaderProgram> ShaderProgram::createFromGlslFiles(
  std::shared_ptr<Device> const&                                  device,
  std::unordered_map<vk::ShaderStageFlagBits, std::string> const& files) {

  std::vector<std::shared_ptr<Illusion::Graphics::ShaderModule>> modules;

  for (auto const& file : files) {
    auto glsl = Illusion::Core::File<std::string>(file.second).getContent();
    modules.push_back(std::make_shared<Illusion::Graphics::ShaderModule>(device, glsl, file.first));
  }

  return std::make_shared<Illusion::Graphics::ShaderProgram>(device, modules);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::ShaderProgram(
  std::shared_ptr<Device> const& device, std::vector<std::shared_ptr<ShaderModule>> const& modules)
  : mDevice(device)
  , mModules(modules) {

  ILLUSION_TRACE << "Creating ShaderProgram." << std::endl;

  createReflection();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::~ShaderProgram() { ILLUSION_TRACE << "Deleting ShaderProgram." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<ShaderModule>> const& ShaderProgram::getModules() const {
  return mModules;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<PipelineReflection> const& ShaderProgram::getReflection() const {
  return mReflection;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<uint32_t, std::shared_ptr<DescriptorSetReflection>> const& ShaderProgram::
  getDescriptorSetReflections() const {
  return mReflection->getDescriptorSetReflections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createReflection() {
  mReflection = std::make_shared<Illusion::Graphics::PipelineReflection>(mDevice);

  for (auto const& module : mModules) {
    for (auto const& resource : module->getResources()) {
      mReflection->addResource(resource);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
