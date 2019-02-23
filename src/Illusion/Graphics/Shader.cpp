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
#include <utility>

namespace Illusion::Graphics {

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

// Map GLSL file extensions to vk::ShaderStageFlagBits.
const std::unordered_map<std::string, vk::ShaderStageFlagBits> glslExtensionMapping = {
    {".frag", vk::ShaderStageFlagBits::eFragment}, {".vert", vk::ShaderStageFlagBits::eVertex},
    {".geom", vk::ShaderStageFlagBits::eGeometry}, {".comp", vk::ShaderStageFlagBits::eCompute},
    {".tesc", vk::ShaderStageFlagBits::eTessellationControl},
    {".tese", vk::ShaderStageFlagBits::eTessellationEvaluation}};

////////////////////////////////////////////////////////////////////////////////////////////////////

// Map HLSL file extensions to vk::ShaderStageFlagBits.
const std::unordered_map<std::string, vk::ShaderStageFlagBits> hlslExtensionMapping = {
    {".ps", vk::ShaderStageFlagBits::eFragment}, {".vs", vk::ShaderStageFlagBits::eVertex},
    {".gs", vk::ShaderStageFlagBits::eGeometry}, {".cs", vk::ShaderStageFlagBits::eCompute},
    {".hs", vk::ShaderStageFlagBits::eTessellationControl},
    {".ds", vk::ShaderStageFlagBits::eTessellationEvaluation}};

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderPtr Shader::createFromFiles(std::string const& name, DeviceConstPtr const& device,
    std::vector<std::string> const& fileNames, std::set<std::string> const& dynamicBuffers,
    bool reloadOnChanges) {

  // First create a new Shader.
  auto shader = Shader::create(name, device);

  // Then attach a module for each given file name.
  for (auto const& fileName : fileNames) {
    auto extension = fileName.substr(fileName.find_last_of('.'));

    // First check whether the file has a glsl extension.
    auto stage = glslExtensionMapping.find(extension);
    if (stage != glslExtensionMapping.end()) {
      shader->addModule(stage->second, GlslFile::create(fileName, reloadOnChanges), dynamicBuffers);
      continue;
    }

    // If not, check whether the file has a hlsl extension.
    stage = hlslExtensionMapping.find(extension);
    if (stage != hlslExtensionMapping.end()) {
      shader->addModule(stage->second, HlslFile::create(fileName, reloadOnChanges), dynamicBuffers);
      continue;
    }

    // Throw an error if we did not find a supported extension.
    throw std::runtime_error(
        "Failed to add shader stage: File " + fileName + " has an unknown extension!");
  }

  return shader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::Shader(std::string const& name, DeviceConstPtr device)
    : Core::NamedObject(name)
    , mDevice(std::move(device)) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::~Shader() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////

void Shader::addModule(vk::ShaderStageFlagBits stage, ShaderSourcePtr const& source,
    std::set<std::string> const& dynamicBuffers) {
  mDirty                 = true;
  mSources[stage]        = source;
  mDynamicBuffers[stage] = dynamicBuffers;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderModuleConstPtr> const& Shader::getModules() const {
  reload();
  return mModules;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineReflectionConstPtr const& Shader::getReflection() const {
  reload();
  return mReflection;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<DescriptorSetReflectionConstPtr> Shader::getDescriptorSetReflections() const {
  reload();
  return mReflection->getDescriptorSetReflections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Shader::reload() const {
  // A new module was added. Just recreate everything. This could be optimized to just recreate the
  // newly added modules, however there will be only very few cases were this method is called
  // before all modules are added anyways.
  if (mDirty) {
    std::vector<ShaderModuleConstPtr> modules;

    auto reflection =
        std::make_shared<PipelineReflection>("PipelineReflection for " + getName(), mDevice);

    try {

      // create modules
      for (auto const& s : mSources) {
        modules.emplace_back(ShaderModule::create("ShaderModule of " + getName(), mDevice, s.second,
            s.first, mDynamicBuffers.at(s.first)));
      }

      // create reflection
      for (auto const& module : modules) {
        for (auto const& resource : module->getResources()) {
          reflection->addResource(resource);
        }
      }
    } catch (std::runtime_error const& e) {
      Core::Logger::error() << "Failed to compile shader: " << e.what() << std::endl;

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
