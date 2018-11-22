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
#include "DescriptorSetCache.hpp"

#include "Context.hpp"
#include "DescriptorPool.hpp"

#include <functional>

namespace Illusion::Graphics {

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

DescriptorSetCache::DescriptorSetCache(std::shared_ptr<Context> const& context)
  : mContext(context) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorSetCache::~DescriptorSetCache() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorSetLayout> DescriptorSetCache::createDescriptorSetLayout(
  SetResources const& setResources) const {

  auto const& hash = setResources.getHash();

  auto it = mCache.find(hash);
  if (it != mCache.end()) { return it->second.mLayout; }

  std::vector<vk::DescriptorSetLayoutBinding> bindings;

  for (auto const& r : setResources.getResources()) {
    auto t = r.second.mResourceType;
    bindings.push_back(
      {r.second.mBinding, resourceTypeMapping.at(t), r.second.mArraySize, r.second.mStages});
  }

  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  descriptorSetLayoutInfo.pBindings    = bindings.data();

  auto descriptorSetLayout = mContext->createDescriptorSetLayout(descriptorSetLayoutInfo);

  CacheEntry entry;
  entry.mLayout = descriptorSetLayout;
  entry.mPool   = std::make_shared<DescriptorPool>(
    mContext, descriptorSetLayoutInfo, descriptorSetLayout, setResources.getSet());

  mCache[hash] = entry;

  return descriptorSetLayout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<DescriptorSet> DescriptorSetCache::acquireHandle(
  std::shared_ptr<vk::DescriptorSetLayout> const& layout) {

  for (auto& cacheEntry : mCache) {
    if (cacheEntry.second.mLayout == layout) {

      // Firts case: we have a handle which has been acquired before; return it!
      if (cacheEntry.second.mFreeHandels.size() > 0) {
        auto descriptorSet = *cacheEntry.second.mFreeHandels.begin();
        cacheEntry.second.mFreeHandels.erase(cacheEntry.second.mFreeHandels.begin());
        cacheEntry.second.mUsedHandels.insert(descriptorSet);
        return descriptorSet;
      }

      // Second case: we have no free handle. So we have to create a new one!
      auto descriptorSet = cacheEntry.second.mPool->allocateDescriptorSet();
      cacheEntry.second.mUsedHandels.insert(descriptorSet);
      return descriptorSet;
    }
  }

  throw std::runtime_error("Failed to acquire DescriptorSet: DescriptorSetLayout was not created "
                           "from this DescriptorSetCache!");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorSetCache::releaseHandle(std::shared_ptr<DescriptorSet> const& handle) {
  for (auto& p : mCache) {
    auto it = p.second.mUsedHandels.find(handle);
    if (it != p.second.mUsedHandels.end()) {}
    p.second.mFreeHandels.insert(p.second.mUsedHandels.begin(), p.second.mUsedHandels.end());
    p.second.mUsedHandels.clear();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorSetCache::releaseAll() {
  for (auto& p : mCache) {
    p.second.mFreeHandels.insert(p.second.mUsedHandels.begin(), p.second.mUsedHandels.end());
    p.second.mUsedHandels.clear();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorSetCache::deleteAll() { mCache.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
