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
#include "Context.hpp"
#include "DescriptorSetCache.hpp"
#include "ShaderModule.hpp"
#include "ShaderReflection.hpp"
#include "Window.hpp"

#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ShaderProgram> ShaderProgram::createFromGlslFiles(
  std::shared_ptr<Context> const&                                 context,
  std::shared_ptr<DescriptorSetCache> const&                      descriptorSetCache,
  std::unordered_map<vk::ShaderStageFlagBits, std::string> const& files) {

  std::vector<std::shared_ptr<Illusion::Graphics::ShaderModule>> modules;

  for (auto const& file : files) {
    auto glsl = Illusion::Core::File<std::string>(file.second).getContent();
    modules.push_back(
      std::make_shared<Illusion::Graphics::ShaderModule>(context, glsl, file.first));
  }

  return std::make_shared<Illusion::Graphics::ShaderProgram>(context, descriptorSetCache, modules);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::ShaderProgram(
  std::shared_ptr<Context> const&                   context,
  std::shared_ptr<DescriptorSetCache> const&        descriptorSetCache,
  std::vector<std::shared_ptr<ShaderModule>> const& modules)
  : mContext(context)
  , mModules(modules) {

  ILLUSION_TRACE << "Creating ShaderProgram." << std::endl;

  createReflection();
  createDescriptorSetLayouts(descriptorSetCache);
  createPipelineLayout();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::~ShaderProgram() {
  ILLUSION_TRACE << "Deleting ShaderProgram." << std::endl;
  mContext->getDevice()->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<ShaderModule>> const& ShaderProgram::getModules() const {
  return mModules;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ShaderReflection> const& ShaderProgram::getReflection() const {
  return mReflection;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<uint32_t, std::shared_ptr<vk::DescriptorSetLayout>> const& ShaderProgram::
  getDescriptorSetLayouts() const {
  return mDescriptorSetLayouts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::PipelineLayout> const& ShaderProgram::getPipelineLayout() const {
  return mPipelineLayout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createReflection() {
  mReflection = std::make_shared<Illusion::Graphics::ShaderReflection>();

  for (auto const& module : mModules) {
    for (auto const& resource : module->getReflection().getResources()) {
      mReflection->addResource(resource.second);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createDescriptorSetLayouts(
  std::shared_ptr<DescriptorSetCache> const& descriptorSetCache) {
  for (auto const& set : mReflection->getSetResources()) {
    mDescriptorSetLayouts[set.first] = descriptorSetCache->createDescriptorSetLayout(set.second);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createPipelineLayout() {
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  for (auto const& descriptorSetLayout : mDescriptorSetLayouts) {
    descriptorSetLayouts.push_back(*descriptorSetLayout.second);
  }

  std::vector<vk::PushConstantRange> pushConstantRanges;
  for (auto const& r :
       mReflection->getResources(PipelineResource::ResourceType::ePushConstantBuffer)) {
    if (r.second.mStages) {
      pushConstantRanges.push_back({r.second.mStages, r.second.mOffset, r.second.mSize});
    }
  }

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
  pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
  pipelineLayoutInfo.pPushConstantRanges    = pushConstantRanges.data();

  mPipelineLayout = mContext->createPipelineLayout(pipelineLayoutInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
