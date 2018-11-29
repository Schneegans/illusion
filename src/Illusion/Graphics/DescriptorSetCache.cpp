////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DescriptorSetCache.hpp"

#include "DescriptorPool.hpp"
#include "Device.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorSetCache::DescriptorSetCache(std::shared_ptr<Device> const& device)
  : mDevice(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorSetCache::~DescriptorSetCache() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<DescriptorSet> DescriptorSetCache::acquireHandle(
  std::shared_ptr<DescriptorSetReflection> const& reflection) {

  auto const& hash       = reflection->getHash();
  auto        cacheEntry = mCache.find(hash);

  if (cacheEntry != mCache.end()) {

    // First case: we have a handle which has been acquired before; return it!
    if (cacheEntry->second.mFreeHandels.size() > 0) {
      auto descriptorSet = *cacheEntry->second.mFreeHandels.begin();
      cacheEntry->second.mFreeHandels.erase(cacheEntry->second.mFreeHandels.begin());
      cacheEntry->second.mUsedHandels.insert(descriptorSet);
      return descriptorSet;
    }

    // Second case: we have no free handle. So we have to create a new one!
    auto descriptorSet = cacheEntry->second.mPool->allocateDescriptorSet();
    cacheEntry->second.mUsedHandels.insert(descriptorSet);
    return descriptorSet;
  }

  // Last case: there is no pool at all! Create a new one!
  CacheEntry entry;
  entry.mPool = std::make_shared<DescriptorPool>(mDevice, reflection);

  auto descriptorSet = entry.mPool->allocateDescriptorSet();
  entry.mUsedHandels.insert(descriptorSet);

  mCache.emplace(hash, entry);

  return descriptorSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorSetCache::releaseHandle(std::shared_ptr<DescriptorSet> const& handle) {

  // Search the cache entry which created this handle
  for (auto& p : mCache) {
    auto it = p.second.mUsedHandels.find(handle);

    // Once found, mark the handle as beeing free agin
    if (it != p.second.mUsedHandels.end()) {
      p.second.mUsedHandels.erase(it);
      p.second.mFreeHandels.insert(handle);
      return;
    }
  }

  throw std::runtime_error("Failed to release descriptor set from DescriptorSetCache: The given "
                           "handle has not been released before or has never been created by this "
                           "cache!");
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
