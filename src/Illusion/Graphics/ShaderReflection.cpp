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
    resource.mResourceType == PipelineResource::ResourceType::OUTPUT ||
    resource.mResourceType == PipelineResource::ResourceType::INPUT)
    key = std::to_string(static_cast<VkShaderStageFlags>(resource.mStages)) + ":" + key;

  // If resource already exists in pipeline resource map, add current stage's bit.
  // Else create a new entry in the pipeline resource map.
  auto it = mResources.find(key);
  if (it != mResources.end())
    it->second.mStages |= resource.mStages;
  else
    mResources.emplace(key, resource);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::addResources(std::vector<PipelineResource> const& resources) {
  for (auto const& resource : resources) {
    addResource(resource);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::printInfo() const {
  const std::unordered_map<PipelineResource::BaseType, std::string> baseTypes = {
    {PipelineResource::BaseType::BOOL, "bool"},
    {PipelineResource::BaseType::CHAR, "char"},
    {PipelineResource::BaseType::INT, "int"},
    {PipelineResource::BaseType::UINT, "uint"},
    {PipelineResource::BaseType::UINT64, "uint64"},
    {PipelineResource::BaseType::HALF, "half"},
    {PipelineResource::BaseType::FLOAT, "float"},
    {PipelineResource::BaseType::DOUBLE, "double"},
    {PipelineResource::BaseType::STRUCT, "struct"},
    {PipelineResource::BaseType::NONE, "none"}};

  const std::unordered_map<PipelineResource::ResourceType, std::string> resourceTypes = {
    {PipelineResource::ResourceType::INPUT, "input"},
    {PipelineResource::ResourceType::OUTPUT, "output"},
    {PipelineResource::ResourceType::SAMPLER, "sampler"},
    {PipelineResource::ResourceType::COMBINED_IMAGE_SAMPLER, "combined_image_sampler"},
    {PipelineResource::ResourceType::SAMPLED_IMAGE, "sampled_image"},
    {PipelineResource::ResourceType::STORAGE_IMAGE, "storage_image"},
    {PipelineResource::ResourceType::UNIFORM_TEXEL_BUFFER, "uniform_texel_buffer"},
    {PipelineResource::ResourceType::STORAGE_TEXEL_BUFFER, "storage_texel_buffer"},
    {PipelineResource::ResourceType::UNIFORM_BUFFER, "uniform_buffer"},
    {PipelineResource::ResourceType::STORAGE_BUFFER, "storage_buffer"},
    {PipelineResource::ResourceType::INPUT_ATTACHMENT, "input_attachment"},
    {PipelineResource::ResourceType::PUSH_CONSTANT_BUFFER, "push_constant_buffer"},
    {PipelineResource::ResourceType::NONE, "none"}};

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
