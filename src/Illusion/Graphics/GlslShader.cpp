////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GlslShader.hpp"

#include "../Core/File.hpp"
#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "PipelineReflection.hpp"
#include "ShaderModule.hpp"
#include "Window.hpp"

#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

GlslShader::GlslShader(DevicePtr const& device, std::vector<std::string> const& fileNames,
    std::set<std::string> const& dynamicBuffers)
    : mDynamicBuffers(dynamicBuffers)
    , mFileNames(fileNames) {

  mDevice = device;

  ILLUSION_TRACE << "Creating GlslShader." << std::endl;

  loadFromFiles();
  createReflection();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GlslShader::~GlslShader() {
  ILLUSION_TRACE << "Deleting GlslShader." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderModulePtr> const& GlslShader::getModules() {
  reload();
  return Shader::getModules();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineReflectionPtr const& GlslShader::getReflection() {
  reload();
  return Shader::getReflection();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<DescriptorSetReflectionPtr> const& GlslShader::getDescriptorSetReflections() {
  reload();
  return Shader::getDescriptorSetReflections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GlslShader::loadFromFiles() {

  static const std::unordered_map<std::string, vk::ShaderStageFlagBits> extensionMapping = {
      {"frag", vk::ShaderStageFlagBits::eFragment}, {"vert", vk::ShaderStageFlagBits::eVertex},
      {"geom", vk::ShaderStageFlagBits::eGeometry}, {"comp", vk::ShaderStageFlagBits::eCompute},
      {"tesc", vk::ShaderStageFlagBits::eTessellationControl},
      {"tese", vk::ShaderStageFlagBits::eTessellationEvaluation}};

  std::vector<ShaderModulePtr>         modules;
  std::vector<Core::File<std::string>> allSourceFiles;

  for (auto const& fileName : mFileNames) {
    auto extension = fileName.substr(fileName.size() - 4);

    auto stage = extensionMapping.find(extension);
    if (stage == extensionMapping.end()) {
      throw std::runtime_error(
          "Failed to add shader stage: File " + fileName + " has an unknown extension!");
    }

    try {
      Illusion::Core::File<std::string> file(fileName);
      modules.emplace_back(std::make_shared<ShaderModule>(
          mDevice, file.getContent(), stage->second, mDynamicBuffers));
      allSourceFiles.emplace_back(file);
    } catch (std::runtime_error const& e) {
      ILLUSION_ERROR << e.what() << std::endl;
      return;
    }
  }

  mModules        = modules;
  mAllSourceFiles = allSourceFiles;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GlslShader::reload() {
  for (auto const& f : mAllSourceFiles) {
    if (f.changedOnDisc()) {
      loadFromFiles();
      createReflection();
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
