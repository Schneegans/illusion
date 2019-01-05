////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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

////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::Shader(DevicePtr const& device, std::vector<ShaderModulePtr> const& modules)
    : mDevice(device)
    , mModules(modules) {

  ILLUSION_TRACE << "Creating Shader." << std::endl;

  createReflection();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::~Shader() {
  ILLUSION_TRACE << "Deleting Shader." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderModulePtr> const& Shader::getModules() {
  return mModules;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineReflectionPtr const& Shader::getReflection() {
  return mReflection;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<DescriptorSetReflectionPtr> const& Shader::getDescriptorSetReflections() {
  return mReflection->getDescriptorSetReflections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Shader::createReflection() {
  mReflection = std::make_shared<PipelineReflection>(mDevice);

  for (auto const& module : mModules) {
    for (auto const& resource : module->getResources()) {
      mReflection->addResource(resource);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
