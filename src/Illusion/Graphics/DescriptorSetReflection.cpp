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
#include "DescriptorSetReflection.hpp"

#include "../Core/Logger.hpp"
#include "Context.hpp"

#include <functional>
#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::unordered_map<PipelineResource::ResourceType, vk::DescriptorType> resourceTypeMapping = {
  {PipelineResource::ResourceType::eCombinedImageSampler,
   vk::DescriptorType::eCombinedImageSampler},
  {PipelineResource::ResourceType::eInputAttachment, vk::DescriptorType::eInputAttachment},
  {PipelineResource::ResourceType::eSampledImage, vk::DescriptorType::eSampledImage},
  {PipelineResource::ResourceType::eSampler, vk::DescriptorType::eSampler},
  {PipelineResource::ResourceType::eStorageBuffer, vk::DescriptorType::eStorageBuffer},
  {PipelineResource::ResourceType::eStorageImage, vk::DescriptorType::eStorageImage},
  {PipelineResource::ResourceType::eStorageTexelBuffer, vk::DescriptorType::eStorageTexelBuffer},
  {PipelineResource::ResourceType::eUniformBuffer, vk::DescriptorType::eUniformBuffer},
  {PipelineResource::ResourceType::eUniformTexelBuffer, vk::DescriptorType::eUniformTexelBuffer},
};

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorSetReflection::DescriptorSetReflection(
  std::shared_ptr<Context> const& context, uint32_t set)
  : mContext(context)
  , mSet(set) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorSetReflection::~DescriptorSetReflection() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorSetReflection::addResource(PipelineResource const& resource) {
  // sanity check
  if (
    resource.mResourceType == PipelineResource::ResourceType::eInput ||
    resource.mResourceType == PipelineResource::ResourceType::eOutput ||
    resource.mResourceType == PipelineResource::ResourceType::eInputAttachment ||
    resource.mResourceType == PipelineResource::ResourceType::ePushConstantBuffer) {

    throw std::runtime_error(
      "Failed to add resource to DescriptorSetReflection: Types Input, Output, "
      "InputAttachment and PushConstantBuffer are not allowed.");
  }

  if (resource.mSet != mSet) {
    throw std::runtime_error(
      "Failed to add resource to DescriptorSetReflection: resource does not belong to this set.");
  }

  mLayout.reset();
  mHash.clear();

  auto it = mResources.find(resource.mName);
  if (it != mResources.end()) {
    it->second.mStages |= resource.mStages;
  } else {
    mResources[resource.mName] = resource;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, PipelineResource> const& DescriptorSetReflection::getResources() const {
  return mResources;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, PipelineResource> DescriptorSetReflection::getResources(
  PipelineResource::ResourceType type) const {

  std::map<std::string, PipelineResource> result;

  for (auto const& r : mResources) {
    if (r.second.mResourceType == type) { result[r.second.mName] = r.second; }
  }

  return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t DescriptorSetReflection::getSet() const { return mSet; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorSetLayout> DescriptorSetReflection::getLayout() const {
  if (!mLayout) {

    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    for (auto const& r : mResources) {
      auto t = r.second.mResourceType;
      bindings.push_back(
        {r.second.mBinding, resourceTypeMapping.at(t), r.second.mArraySize, r.second.mStages});
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutInfo.pBindings    = bindings.data();

    mLayout = mContext->createDescriptorSetLayout(descriptorSetLayoutInfo);
  }

  return mLayout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Core::BitHash const& DescriptorSetReflection::getHash() const {
  if (mHash.size() == 0) {
    mHash.push<32>(mSet);

    for (auto const& r : mResources) {
      // TODO: Are those bit sizes too much / sufficient? These are based on Vulkan-EZ
      mHash.push<6>(r.second.mStages);
      mHash.push<4>(r.second.mResourceType);
      mHash.push<16>(r.second.mBinding);
      mHash.push<32>(r.second.mArraySize);
    }
  }

  return mHash;
}
} // namespace Illusion::Graphics
