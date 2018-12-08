////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

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

const std::unordered_map<std::string, vk::ShaderStageFlagBits> extensionMapping = {
  {"frag", vk::ShaderStageFlagBits::eFragment}, {"vert", vk::ShaderStageFlagBits::eVertex},
  {"geom", vk::ShaderStageFlagBits::eGeometry}, {"comp", vk::ShaderStageFlagBits::eCompute},
  {"tesc", vk::ShaderStageFlagBits::eTessellationControl},
  {"tese", vk::ShaderStageFlagBits::eTessellationEvaluation}};

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgramPtr ShaderProgram::createFromFiles(
  DevicePtr const& device, std::vector<std::string> const& files) {

  std::vector<ShaderModulePtr> modules;

  for (auto const& file : files) {
    auto extension = file.substr(file.size() - 4);

    auto stage = extensionMapping.find(extension);
    if (stage == extensionMapping.end()) {
      throw std::runtime_error(
        "Failed to add shader stage: File " + file + " has an unknown extension!");
    }

    auto glsl = Illusion::Core::File<std::string>(file).getContent();
    modules.push_back(std::make_shared<ShaderModule>(device, glsl, stage->second));
  }

  return std::make_shared<ShaderProgram>(device, modules);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::ShaderProgram(DevicePtr const& device, std::vector<ShaderModulePtr> const& modules)
  : mDevice(device)
  , mModules(modules) {

  ILLUSION_TRACE << "Creating ShaderProgram." << std::endl;

  createReflection();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::~ShaderProgram() { ILLUSION_TRACE << "Deleting ShaderProgram." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderModulePtr> const& ShaderProgram::getModules() const { return mModules; }

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineReflectionPtr const& ShaderProgram::getReflection() const { return mReflection; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<uint32_t, DescriptorSetReflectionPtr> const&
ShaderProgram::getDescriptorSetReflections() const {
  return mReflection->getDescriptorSetReflections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createReflection() {
  mReflection = std::make_shared<PipelineReflection>(mDevice);

  for (auto const& module : mModules) {
    for (auto const& resource : module->getResources()) {
      mReflection->addResource(resource);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
