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
#include "PipelineLayout.hpp"

#include "../Core/File.hpp"
#include "../Core/Logger.hpp"
#include "Engine.hpp"
#include "RenderPass.hpp"
#include "ShaderReflection.hpp"
#include "Window.hpp"

#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineLayout::PipelineLayout(
  std::shared_ptr<Engine> const&  engine,
  std::vector<std::string> const& shaderFiles,
  uint32_t                        descriptorCount)
  : mEngine(engine)
  , mShaderFiles(shaderFiles)
  , mDescriptorCount(descriptorCount) {

  ILLUSION_TRACE << "Creating PipelineLayout." << std::endl;

  mShaderCodes          = loadShaderCodes();
  mStageReflections     = createStageReflections();
  mProgramReflection    = createProgramReflection();
  mDescriptorPool       = createDescriptorPool();
  mDescriptorSetLayouts = createDescriptorSetLayouts();
  mPipelineLayout       = createPipelineLayout();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineLayout::~PipelineLayout() {
  ILLUSION_TRACE << "Deleting PipelineLayout." << std::endl;
  mEngine->getDevice()->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PipelineLayout::useDescriptorSet(
  vk::CommandBuffer const& cmd, vk::DescriptorSet const& descriptorSet, int set) const {
  cmd.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, *mPipelineLayout, set, descriptorSet, nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PipelineLayout::setPushConstant(
  vk::CommandBuffer const& cmd,
  vk::ShaderStageFlags     stages,
  uint32_t                 size,
  uint8_t*                 data,
  uint32_t                 offset) const {

  cmd.pushConstants(*mPipelineLayout, stages, offset, size, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorSet PipelineLayout::allocateDescriptorSet(int set) const {
  if (!mDescriptorPool) {
    throw std::runtime_error{"Cannot allocated DescriptorSet: DescriptorSetLayout is empty!"};
  }

  vk::DescriptorSetLayout       descriptorSetLayouts[] = {*mDescriptorSetLayouts[set]};
  vk::DescriptorSetAllocateInfo info;
  info.descriptorPool     = *mDescriptorPool;
  info.descriptorSetCount = 1;
  info.pSetLayouts        = descriptorSetLayouts;

  return mEngine->getDevice()->allocateDescriptorSets(info)[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PipelineLayout::freeDescriptorSet(vk::DescriptorSet const& set) const {
  if (!mDescriptorPool) {
    throw std::runtime_error{"Cannot free DescriptorSet: DescriptorSetLayout is empty!"};
  }

  mEngine->getDevice()->freeDescriptorSets(*mDescriptorPool, set);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::vector<uint32_t>> PipelineLayout::loadShaderCodes() const {
  std::vector<std::vector<uint32_t>> shaderCodes(mShaderFiles.size());

  for (size_t i{0}; i < mShaderFiles.size(); ++i) {
    shaderCodes[i] = Core::File<uint32_t>(mShaderFiles[i]).getContent();
  }

  return shaderCodes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<ShaderReflection>> PipelineLayout::createStageReflections() const {
  std::vector<std::shared_ptr<ShaderReflection>> reflections(mShaderCodes.size());
  for (size_t i{0}; i < mShaderCodes.size(); ++i) {
    try {
      reflections[i] = std::make_shared<ShaderReflection>(mShaderCodes[i]);
    } catch (std::runtime_error const& e) {
      throw std::runtime_error{"Failed to get reflection information for " + mShaderFiles[i] +
                               ": " + e.what()};
    }
  }
  return reflections;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ShaderReflection> PipelineLayout::createProgramReflection() const {
  try {
    return std::make_shared<ShaderReflection>(mStageReflections);
  } catch (std::runtime_error const& e) {
    std::string files;
    for (size_t i{0}; i < mShaderFiles.size(); ++i) {
      files += mShaderFiles[i];
      if (i == mShaderFiles.size() - 2)
        files += " and ";
      else if (i < mShaderFiles.size() - 2)
        files += ", ";
    }
    throw std::runtime_error{"Failed to merge reflection information for " + files + ": " +
                             e.what()};
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorPool> PipelineLayout::createDescriptorPool() const {
  std::vector<vk::DescriptorPoolSize> pools;

  if (mProgramReflection->getSamplers().size() > 0) {
    vk::DescriptorPoolSize pool;
    pool.type = vk::DescriptorType::eCombinedImageSampler;
    pool.descriptorCount =
      static_cast<uint32_t>(mProgramReflection->getSamplers().size() * mDescriptorCount);
    pools.push_back(pool);
  }

  if (mProgramReflection->getStorageImages().size() > 0) {
    vk::DescriptorPoolSize pool;
    pool.type = vk::DescriptorType::eStorageImage;
    pool.descriptorCount =
      static_cast<uint32_t>(mProgramReflection->getStorageImages().size() * mDescriptorCount);
    pools.push_back(pool);
  }

  if (mProgramReflection->getUniformBuffers().size() > 0) {
    vk::DescriptorPoolSize pool;
    pool.type = vk::DescriptorType::eUniformBuffer;
    pool.descriptorCount =
      static_cast<uint32_t>(mProgramReflection->getUniformBuffers().size()) * mDescriptorCount;
    pools.push_back(pool);
  }

  if (pools.size() > 0) {
    vk::DescriptorPoolCreateInfo info;
    info.poolSizeCount = static_cast<uint32_t>(pools.size());
    info.pPoolSizes    = pools.data();
    info.maxSets       = mDescriptorCount;
    info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    return mEngine->createDescriptorPool(info);
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<vk::DescriptorSetLayout>> PipelineLayout::createDescriptorSetLayouts()
  const {

  std::vector<std::shared_ptr<vk::DescriptorSetLayout>> descriptorSetLayouts;

  for (auto set : mProgramReflection->getActiveDescriptorSets()) {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    for (auto const& resource : mProgramReflection->getUniformBuffers()) {
      if (resource.mSet == set) {
        bindings.push_back(
          {resource.mBinding, vk::DescriptorType::eUniformBuffer, 1, resource.mActiveStages});
      }
    }

    for (auto const& resource : mProgramReflection->getSamplers()) {
      if (resource.mSet == set) {
        bindings.push_back({resource.mBinding,
                            vk::DescriptorType::eCombinedImageSampler,
                            1,
                            resource.mActiveStages});
      }
    }

    for (auto const& resource : mProgramReflection->getStorageImages()) {
      if (resource.mSet == set) {
        bindings.push_back(
          {resource.mBinding, vk::DescriptorType::eStorageImage, 1, resource.mActiveStages});
      }
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutInfo.pBindings    = bindings.data();

    descriptorSetLayouts.push_back(mEngine->createDescriptorSetLayout(descriptorSetLayoutInfo));
  }

  return descriptorSetLayouts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::PipelineLayout> PipelineLayout::createPipelineLayout() const {
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  for (auto const& descriptorSetLayout : mDescriptorSetLayouts) {
    descriptorSetLayouts.push_back(*descriptorSetLayout.get());
  }

  std::vector<vk::PushConstantRange> pushConstantRanges;
  for (auto const& pushConstant : mProgramReflection->getPushConstantBuffers()) {
    if (pushConstant.mActiveStages) {
      pushConstantRanges.push_back({pushConstant.mActiveStages, 0, pushConstant.mSize});
    }
  }

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
  pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
  pipelineLayoutInfo.pPushConstantRanges    = pushConstantRanges.data();

  return mEngine->createPipelineLayout(pipelineLayoutInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
