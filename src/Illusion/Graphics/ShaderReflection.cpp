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
#include "ShaderReflection.hpp"

#include "../Core/Logger.hpp"

#include <functional>
#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// parts of this code is based on Vulkan-EZ
// (MIT, Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.)
////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::~ShaderReflection() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::addResource(PipelineResource const& resource) {
  // The key used for each resource is its name, except in the case of outputs, since its legal to
  // have separate outputs with the same name across shader stages.
  auto key = resource.mName;
  if (
    resource.mResourceType == PipelineResource::ResourceType::eOutput ||
    resource.mResourceType == PipelineResource::ResourceType::eInput) {

    key = std::to_string(static_cast<VkShaderStageFlags>(resource.mStages)) + ":" + key;
  }

  // If resource already exists in pipeline resource map, add current stage's bit.
  // Else create a new entry in the pipeline resource map.
  auto it = mResources.find(key);
  if (it != mResources.end()) {
    it->second.mStages |= resource.mStages;
  } else {
    mResources.emplace(key, resource);
    mActiveSets.insert(resource.mSet);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::addResources(std::vector<PipelineResource> const& resources) {
  for (auto const& resource : resources) {
    addResource(resource);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, PipelineResource> const& ShaderReflection::getResources() const {
  return mResources;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<PipelineResource> ShaderReflection::getResources(
  PipelineResource::ResourceType type) const {

  std::vector<PipelineResource> result;

  for (auto const& r : mResources) {
    if (r.second.mResourceType == type) { result.push_back(r.second); }
  }

  return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<PipelineResource> ShaderReflection::getResources(uint32_t set) const {

  std::vector<PipelineResource> result;

  for (auto const& r : mResources) {
    if (r.second.mSet == set) { result.push_back(r.second); }
  }

  return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::set<uint32_t> const& ShaderReflection::getActiveSets() const { return mActiveSets; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::printInfo() const {
  const std::unordered_map<PipelineResource::BaseType, std::string> baseTypes = {
    {PipelineResource::BaseType::eBool, "bool"},
    {PipelineResource::BaseType::eChar, "char"},
    {PipelineResource::BaseType::eInt, "int"},
    {PipelineResource::BaseType::eUint, "uint"},
    {PipelineResource::BaseType::eUint64, "uint64"},
    {PipelineResource::BaseType::eHalf, "half"},
    {PipelineResource::BaseType::eFloat, "float"},
    {PipelineResource::BaseType::eDouble, "double"},
    {PipelineResource::BaseType::eStruct, "struct"},
    {PipelineResource::BaseType::eNone, "none"}};

  const std::unordered_map<PipelineResource::ResourceType, std::string> resourceTypes = {
    {PipelineResource::ResourceType::eInput, "input"},
    {PipelineResource::ResourceType::eOutput, "output"},
    {PipelineResource::ResourceType::eSampler, "sampler"},
    {PipelineResource::ResourceType::eCombinedImageSampler, "combined_image_sampler"},
    {PipelineResource::ResourceType::eSampledImage, "sampled_image"},
    {PipelineResource::ResourceType::eStorageImage, "storage_image"},
    {PipelineResource::ResourceType::eUniformTexelBuffer, "uniform_texel_buffer"},
    {PipelineResource::ResourceType::eStorageTexelBuffer, "storage_texel_buffer"},
    {PipelineResource::ResourceType::eUniformBuffer, "uniform_buffer"},
    {PipelineResource::ResourceType::eStorageBuffer, "storage_buffer"},
    {PipelineResource::ResourceType::eInputAttachment, "input_attachment"},
    {PipelineResource::ResourceType::ePushConstantBuffer, "push_constant_buffer"},
    {PipelineResource::ResourceType::eNone, "none"}};

  std::function<void(PipelineResource::Member const&, int)> printMemberInfo =
    [&printMemberInfo, &baseTypes](PipelineResource::Member const& m, int indent) {

      ILLUSION_MESSAGE << std::string(indent * 2, ' ') << "- \"" << m.mName
                       << "\", type: " << baseTypes.find(m.mBaseType)->second
                       << ", dims: " << m.mColumns << "x" << m.mVecSize << "[" << m.mArraySize
                       << "], size: " << m.mSize << ", offset: " << m.mOffset << std::endl;

      for (auto const& member : m.mMembers) {
        printMemberInfo(member, indent + 1);
      }
    };

  for (auto const& pair : mResources) {
    auto const& r = pair.second;
    ILLUSION_MESSAGE << "- \"" << r.mName << "\" (" << resourceTypes.find(r.mResourceType)->second
                     << ", " << vk::to_string(r.mStages) << ", access: " << vk::to_string(r.mAccess)
                     << ", set: " << r.mSet << ", binding: " << r.mBinding
                     << ", location: " << r.mLocation << ")" << std::endl;
    for (auto const& member : r.mMembers) {
      printMemberInfo(member, 1);
    }
  }
}

} // namespace Illusion::Graphics
