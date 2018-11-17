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
#include "ShaderModule.hpp"
#include "ShaderReflection.hpp"
#include "Window.hpp"

#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::ShaderProgram(
  std::shared_ptr<Context> const&                   context,
  std::vector<std::shared_ptr<ShaderModule>> const& modules)
  : mContext(context)
  , mModules(modules) {

  ILLUSION_TRACE << "Creating ShaderProgram." << std::endl;

  createReflection();
  createDescriptorPools();
  createPipelineLayout();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderProgram::~ShaderProgram() {
  ILLUSION_TRACE << "Deleting ShaderProgram." << std::endl;
  mContext->getDevice()->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// void ShaderProgram::useDescriptorSet(
//   vk::CommandBuffer const& cmd, vk::DescriptorSet const& descriptorSet, uint32_t set) const {
//   cmd.bindDescriptorSets(
//     vk::PipelineBindPoint::eGraphics, *mPipelineLayout, set, descriptorSet, nullptr);
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

// void ShaderProgram::setPushConstant(
//   vk::CommandBuffer const& cmd,
//   vk::ShaderStageFlags     stages,
//   uint32_t                 size,
//   uint8_t*                 data,
//   uint32_t                 offset) const {

//   cmd.pushConstants(*mPipelineLayout, stages, offset, size, data);
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorSet> ShaderProgram::allocateDescriptorSet(uint32_t setNum) {
  if (mDescriptorPools.size() <= setNum) {
    throw std::runtime_error(
      "Cannot allocated DescriptorSet: No set number " + std::to_string(setNum) +
      " available in this pipeline!");
  }

  return mDescriptorPools[setNum]->allocateDescriptorSet();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createReflection() {
  mReflection = std::make_shared<Illusion::Graphics::ShaderReflection>();

  for (auto module : mModules) {
    mReflection->addResources(module->getResources());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createDescriptorPools() {
  for (uint32_t set : mReflection->getActiveSets()) {
    mDescriptorPools.push_back(
      std::make_shared<DescriptorPool>(mContext, mReflection->getResources(set)));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgram::createPipelineLayout() {
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  for (auto const& descriptorPool : mDescriptorPools) {
    descriptorSetLayouts.push_back(*descriptorPool->getLayout().get());
  }

  std::vector<vk::PushConstantRange> pushConstantRanges;
  for (auto const& r :
       mReflection->getResources(PipelineResource::ResourceType::ePushConstantBuffer)) {
    if (r.mStages) { pushConstantRanges.push_back({r.mStages, r.mOffset, r.mSize}); }
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
