////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PipelineReflection.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"

#include <functional>
#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineReflection::PipelineReflection(DevicePtr const& device)
    : mDevice(device) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineReflection::~PipelineReflection() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PipelineReflection::addResource(PipelineResource const& resource) {

  mLayout.reset();

  // As in Vulkan-EZ, the key used for each resource is its name, except in the case of inputs and
  // outputs, since its legal to have separate outputs and inputs with the same name across
  // shader stages.
  auto key = resource.mName;
  if (resource.mResourceType == PipelineResource::ResourceType::eOutput ||
      resource.mResourceType == PipelineResource::ResourceType::eInput) {

    key = std::to_string(static_cast<VkShaderStageFlags>(resource.mStages)) + ":" + key;
  }

  if (resource.mResourceType == PipelineResource::ResourceType::eInput ||
      resource.mResourceType == PipelineResource::ResourceType::eOutput ||
      resource.mResourceType == PipelineResource::ResourceType::ePushConstantBuffer) {

    std::map<std::string, PipelineResource>* map;

    if (resource.mResourceType == PipelineResource::ResourceType::eInput) {
      map = &mInputs;
    } else if (resource.mResourceType == PipelineResource::ResourceType::eOutput) {
      map = &mOutputs;
    } else if (resource.mResourceType == PipelineResource::ResourceType::ePushConstantBuffer) {
      map = &mPushConstantBuffers;
    }

    auto it = map->find(key);
    if (it != map->end()) {
      it->second.mStages |= resource.mStages;
    } else {
      map->emplace(key, resource);
    }

    return;
  }

  while (mDescriptorSetReflections.size() <= resource.mSet) {
    mDescriptorSetReflections.emplace_back(
        std::make_shared<DescriptorSetReflection>(mDevice, mDescriptorSetReflections.size()));
  }

  mDescriptorSetReflections[resource.mSet]->addResource(resource);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<DescriptorSetReflectionPtr> const&
PipelineReflection::getDescriptorSetReflections() const {
  return mDescriptorSetReflections;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, PipelineResource> PipelineReflection::getResources(
    PipelineResource::ResourceType type) const {

  if (type == PipelineResource::ResourceType::eInput) {
    return mInputs;
  } else if (type == PipelineResource::ResourceType::eOutput) {
    return mOutputs;
  } else if (type == PipelineResource::ResourceType::ePushConstantBuffer) {
    return mPushConstantBuffers;
  }

  std::map<std::string, PipelineResource> result;

  for (auto const& s : mDescriptorSetReflections) {
    auto const& r = s->getResources(type);
    result.insert(r.begin(), r.end());
  }

  return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, PipelineResource> PipelineReflection::getResources() const {

  std::map<std::string, PipelineResource> result;

  for (auto const& s : mDescriptorSetReflections) {
    auto const& r = s->getResources();
    result.insert(r.begin(), r.end());
  }

  result.insert(mInputs.begin(), mInputs.end());
  result.insert(mOutputs.begin(), mOutputs.end());
  result.insert(mPushConstantBuffers.begin(), mPushConstantBuffers.end());

  return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PipelineLayoutPtr const& PipelineReflection::getLayout() const {
  if (!mLayout) {
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    for (auto const& r : mDescriptorSetReflections) {
      descriptorSetLayouts.push_back(*r->getLayout());
    }

    std::vector<vk::PushConstantRange> pushConstantRanges;
    for (auto const& r : mPushConstantBuffers) {
      if (r.second.mStages) {
        pushConstantRanges.push_back({r.second.mStages, r.second.mOffset, r.second.mSize});
      }
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount         = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges    = pushConstantRanges.data();

    mLayout = mDevice->createPipelineLayout(pipelineLayoutInfo);
  }

  return mLayout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PipelineReflection::printInfo() const {
  ILLUSION_MESSAGE << "Inputs" << std::endl;
  for (auto const& r : mInputs) {
    ILLUSION_MESSAGE << "  - \"" << r.second.mName << "\" (" << vk::to_string(r.second.mStages)
                     << ", binding: " << r.second.mBinding << ", location: " << r.second.mLocation
                     << ")" << std::endl;
  }

  ILLUSION_MESSAGE << "Outputs" << std::endl;
  for (auto const& r : mOutputs) {
    ILLUSION_MESSAGE << "  - \"" << r.second.mName << "\" (" << vk::to_string(r.second.mStages)
                     << ", binding: " << r.second.mBinding << ", location: " << r.second.mLocation
                     << ")" << std::endl;
  }

  ILLUSION_MESSAGE << "PushConstants" << std::endl;
  for (auto const& r : mPushConstantBuffers) {
    ILLUSION_MESSAGE << "  - \"" << r.second.mName << "\" (" << vk::to_string(r.second.mStages)
                     << ", size: " << r.second.mSize << ", offset: " << r.second.mOffset << ")"
                     << std::endl;
  }

  for (auto const& s : mDescriptorSetReflections) {
    s->printInfo();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
