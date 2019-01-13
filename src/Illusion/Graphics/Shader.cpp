////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Shader.hpp"

#include "../Core/File.hpp"
#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "PipelineReflection.hpp"
#include "ShaderModule.hpp"
#include "Window.hpp"

#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

namespace {
const std::unordered_map<std::string, vk::ShaderStageFlagBits> glslExtensionMapping = {
    {".frag", vk::ShaderStageFlagBits::eFragment}, {".vert", vk::ShaderStageFlagBits::eVertex},
    {".geom", vk::ShaderStageFlagBits::eGeometry}, {".comp", vk::ShaderStageFlagBits::eCompute},
    {".tesc", vk::ShaderStageFlagBits::eTessellationControl},
    {".tese", vk::ShaderStageFlagBits::eTessellationEvaluation}};

const std::unordered_map<std::string, vk::ShaderStageFlagBits> hlslExtensionMapping = {
    {".ps", vk::ShaderStageFlagBits::eFragment}, {".vs", vk::ShaderStageFlagBits::eVertex},
    {".gs", vk::ShaderStageFlagBits::eGeometry}, {".cs", vk::ShaderStageFlagBits::eCompute},
    {".hs", vk::ShaderStageFlagBits::eTessellationControl},
    {".ds", vk::ShaderStageFlagBits::eTessellationEvaluation}};
} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderPtr Shader::createFromFiles(DevicePtr const& device,
    std::vector<std::string> const& fileNames, std::set<std::string> dynamicBuffers,
    bool reloadOnChanges) {

  auto shader = Shader::create(device);

  for (auto const& fileName : fileNames) {
    auto extension = fileName.substr(fileName.find_last_of('.'));

    // first check whether the file has a glsl extension
    auto stage = glslExtensionMapping.find(extension);
    if (stage != glslExtensionMapping.end()) {
      shader->addModule(stage->second, GlslFile::create(fileName, reloadOnChanges), dynamicBuffers);
      continue;
    }

    // then check whether the file has a hlsl extension
    stage = hlslExtensionMapping.find(extension);
    if (stage != hlslExtensionMapping.end()) {
      shader->addModule(stage->second, HlslFile::create(fileName, reloadOnChanges), dynamicBuffers);
      continue;
    }

    // throw an error if did not find a supported extension
    throw std::runtime_error(
        "Failed to add shader stage: File " + fileName + " has an unknown extension!");
  }

  return shader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::Shader(DevicePtr const& device)
    : mDevice(device) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::~Shader() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Shader::addModule(vk::ShaderStageFlagBits stage, ShaderSourcePtr const& source,
    std::set<std::string> const& dynamicBuffers) {
  mDirty                 = true;
  mSources[stage]        = source;
  mDynamicBuffers[stage] = dynamicBuffers;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderModulePtr> const& Shader::getModules() {
  reload();
  return mModules;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineReflectionPtr const& Shader::getReflection() {
  reload();
  return mReflection;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<DescriptorSetReflectionPtr> const& Shader::getDescriptorSetReflections() {
  reload();
  return mReflection->getDescriptorSetReflections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Shader::reload() {

  // first check whether one of our modules needs to be reload (this is for example the case when
  // the source file changed on disc)
  for (auto const& m : mModules) {
    if (m->requiresReload()) {
      try {
        m->reload();
      } catch (std::runtime_error const& e) {
        ILLUSION_ERROR << "Shader reloading failed. " << e.what() << std::endl;
        m->resetReloadingRequired();
      }
    }
  }

  // A new module was added. Just recreate everything. This could be optimized to just recreate the
  // newly added modules, however there will be only very few cases were this method is called
  // before all modules are added anyways.
  if (mDirty) {
    std::vector<ShaderModulePtr> modules;

    auto reflection = std::make_shared<PipelineReflection>(mDevice);

    try {

      // create modules
      for (auto const& s : mSources) {
        modules.emplace_back(
            ShaderModule::create(mDevice, s.second, s.first, mDynamicBuffers[s.first]));
      }

      // create reflection
      for (auto const& module : modules) {
        for (auto const& resource : module->getResources()) {
          reflection->addResource(resource);
        }
      }
    } catch (std::runtime_error const& e) {
      ILLUSION_ERROR << "Failed to compile shader: " << e.what() << std::endl;

      // setting mDirty to false to prevent multiple error prints
      mDirty = false;
      return;
    }

    mReflection = reflection;
    mModules    = modules;
    mDirty      = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
