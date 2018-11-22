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
#include "SetResources.hpp"

#include "../Core/Logger.hpp"

#include <functional>
#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

SetResources::SetResources(uint32_t set)
  : mSet(set) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

SetResources::~SetResources() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SetResources::addResource(PipelineResource const& resource) {
  // sanity check
  if (
    resource.mResourceType == PipelineResource::ResourceType::eInput ||
    resource.mResourceType == PipelineResource::ResourceType::eOutput ||
    resource.mResourceType == PipelineResource::ResourceType::eInputAttachment ||
    resource.mResourceType == PipelineResource::ResourceType::ePushConstantBuffer) {

    throw std::runtime_error("Failed to add resource to SetResources: Types Input, Output, "
                             "InputAttachment and PushConstantBuffer are not allowed.");
  }

  if (mResources.begin()->second.mSet != mSet) {
    throw std::runtime_error(
      "Failed to add resource to SetResources: resource does not belong to this set.");
  }

  auto it = mResources.find(resource.mName);
  if (it != mResources.end()) {
    it->second.mStages |= resource.mStages;
  } else {
    mResources[resource.mName] = resource;
  }

  mHashDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, PipelineResource> const& SetResources::getResources() const {
  return mResources;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, PipelineResource> SetResources::getResources(
  PipelineResource::ResourceType type) const {

  std::map<std::string, PipelineResource> result;

  for (auto const& r : mResources) {
    if (r.second.mResourceType == type) { result[r.second.mName] = r.second; }
  }

  return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t SetResources::getSet() const { return mSet; }

////////////////////////////////////////////////////////////////////////////////////////////////////

Core::BitHash const& SetResources::getHash() const {
  if (mHashDirty) {
    mHashDirty = false;
    mHash.clear();

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
