////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DescriptorSetReflection.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"

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
    {PipelineResource::ResourceType::eStorageBufferDynamic,
        vk::DescriptorType::eStorageBufferDynamic},
    {PipelineResource::ResourceType::eStorageImage, vk::DescriptorType::eStorageImage},
    {PipelineResource::ResourceType::eStorageTexelBuffer, vk::DescriptorType::eStorageTexelBuffer},
    {PipelineResource::ResourceType::eUniformBuffer, vk::DescriptorType::eUniformBuffer},
    {PipelineResource::ResourceType::eUniformBufferDynamic,
        vk::DescriptorType::eUniformBufferDynamic},
    {PipelineResource::ResourceType::eUniformTexelBuffer, vk::DescriptorType::eUniformTexelBuffer},
};

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorSetReflection::DescriptorSetReflection(
    std::string const& name, DevicePtr const& device, uint32_t set)
    : Core::NamedObject(name)
    , mDevice(device)
    , mSet(set) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorSetReflection::~DescriptorSetReflection() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorSetReflection::addResource(PipelineResource const& resource) {
  // sanity checks
  if (resource.mResourceType == PipelineResource::ResourceType::eInput ||
      resource.mResourceType == PipelineResource::ResourceType::eOutput ||
      resource.mResourceType == PipelineResource::ResourceType::ePushConstantBuffer) {

    throw std::runtime_error(
        "Failed to add resource to DescriptorSetReflection: Types Input, Output and "
        "PushConstantBuffer are not allowed.");
  }

  if (resource.mSet != mSet) {
    throw std::runtime_error(
        "Failed to add resource to DescriptorSetReflection: resource does not belong to this set.");
  }

  // reset lazy members
  mLayout.reset();
  mHash.clear();

  // add resource; if its already there just append it's mStages
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
    if (r.second.mResourceType == type) {
      result[r.second.mName] = r.second;
    }
  }

  return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t DescriptorSetReflection::getSet() const {
  return mSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorSetLayoutPtr DescriptorSetReflection::getLayout() const {
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

    mLayout = mDevice->createDescriptorSetLayout(
        "DescriptorSetLayout for " + getName(), descriptorSetLayoutInfo);
  }

  return mLayout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorSetReflection::printInfo() const {
  const std::unordered_map<PipelineResource::BaseType, std::string> baseTypes = {
      {PipelineResource::BaseType::eBool, "bool"}, {PipelineResource::BaseType::eChar, "char"},
      {PipelineResource::BaseType::eInt, "int"}, {PipelineResource::BaseType::eUint, "uint"},
      {PipelineResource::BaseType::eUint64, "uint64"}, {PipelineResource::BaseType::eHalf, "half"},
      {PipelineResource::BaseType::eFloat, "float"},
      {PipelineResource::BaseType::eDouble, "double"},
      {PipelineResource::BaseType::eStruct, "struct"}, {PipelineResource::BaseType::eNone, "none"}};

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
      {PipelineResource::ResourceType::eUniformBufferDynamic, "uniform_buffer_dynamic"},
      {PipelineResource::ResourceType::eStorageBuffer, "storage_buffer"},
      {PipelineResource::ResourceType::eStorageBufferDynamic, "storage_buffer_dynamic"},
      {PipelineResource::ResourceType::eInputAttachment, "input_attachment"},
      {PipelineResource::ResourceType::ePushConstantBuffer, "push_constant_buffer"},
      {PipelineResource::ResourceType::eNone, "none"}};

  std::function<void(PipelineResource::Member const&, int)> printMemberInfo =
      [&printMemberInfo, &baseTypes](PipelineResource::Member const& m, int indent) {

        Core::Logger::message() << std::string(indent * 2, ' ') << "- \"" << m.mName
                                << "\", type: " << baseTypes.find(m.mBaseType)->second
                                << ", dims: " << m.mColumns << "x" << m.mVecSize << "["
                                << m.mArraySize << "], size: " << m.mSize
                                << ", offset: " << m.mOffset << std::endl;

        for (auto const& member : m.mMembers) {
          printMemberInfo(member, indent + 1);
        }
      };

  Core::Logger::message() << "Set: " << mSet << std::endl;
  for (auto const& pair : mResources) {
    auto const& r = pair.second;
    Core::Logger::message() << "  - \"" << r.mName << "\" ("
                            << resourceTypes.find(r.mResourceType)->second << ", "
                            << vk::to_string(r.mStages) << ", access: " << vk::to_string(r.mAccess)
                            << ", set: " << r.mSet << ", binding: " << r.mBinding
                            << ", location: " << r.mLocation << ")" << std::endl;
    for (auto const& member : r.mMembers) {
      printMemberInfo(member, 2);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Core::BitHash const& DescriptorSetReflection::getHash() const {
  if (mHash.size() == 0) {
    mHash.push<16>(mSet);

    for (auto const& r : mResources) {
      mHash.push<6>(r.second.mStages);
      mHash.push<4>(r.second.mResourceType);
      mHash.push<16>(r.second.mBinding);
      mHash.push<32>(r.second.mArraySize);
    }
  }

  return mHash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
