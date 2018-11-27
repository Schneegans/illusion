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
#include "BufferCache.hpp"

#include "Device.hpp"

#include <functional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

BufferCache::BufferCache(std::shared_ptr<Device> const& device)
  : mDevice(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

BufferCache::~BufferCache() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackedBuffer> BufferCache::acquireHandle(
  vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {

  Core::BitHash hash;
  hash.push<64>(size);
  hash.push<9>(usage);
  hash.push<6>(properties);

  auto cacheEntry = mCache.find(hash);

  if (cacheEntry != mCache.end()) {

    // First case: we have a handle which has been acquired before; return it!
    if (cacheEntry->second.mFreeHandels.size() > 0) {
      auto backedBuffer = *cacheEntry->second.mFreeHandels.begin();
      cacheEntry->second.mFreeHandels.erase(cacheEntry->second.mFreeHandels.begin());
      cacheEntry->second.mUsedHandels.insert(backedBuffer);
      return backedBuffer;
    }

    // Second case: we have no free handle. So we have to create a new one!
    auto backedBuffer = mDevice->createBackedBuffer(size, usage, properties);
    cacheEntry->second.mUsedHandels.insert(backedBuffer);
    return backedBuffer;
  }

  // Last case: there is no cache entry at all! Create a new one!
  CacheEntry entry;
  auto       backedBuffer = mDevice->createBackedBuffer(size, usage, properties);
  entry.mUsedHandels.insert(backedBuffer);
  mCache.emplace(hash, entry);

  return backedBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BufferCache::releaseHandle(std::shared_ptr<BackedBuffer> const& handle) {
  for (auto& p : mCache) {
    auto it = p.second.mUsedHandels.find(handle);
    if (it != p.second.mUsedHandels.end()) {}
    p.second.mFreeHandels.insert(p.second.mUsedHandels.begin(), p.second.mUsedHandels.end());
    p.second.mUsedHandels.clear();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BufferCache::releaseAll() {
  for (auto& p : mCache) {
    p.second.mFreeHandels.insert(p.second.mUsedHandels.begin(), p.second.mUsedHandels.end());
    p.second.mUsedHandels.clear();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BufferCache::deleteAll() { mCache.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
