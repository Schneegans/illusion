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
#include "DescriptorPoolCache.hpp"

#include "DescriptorPool.hpp"

#include <functional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorPoolCache::DescriptorPoolCache(std::shared_ptr<Context> const& context)
  : mContext(context) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorPoolCache::~DescriptorPoolCache() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<DescriptorPool> DescriptorPoolCache::get(
  std::vector<PipelineResource> const& setResources, uint32_t set) {

  Core::BitHash hash;

  std::function<void(PipelineResource::Member const&)> pushMember =
    [&hash, &pushMember](PipelineResource::Member const& r) {
      hash.push<4>(r.mBaseType);
      hash.push<32>(r.mOffset);
      hash.push<32>(r.mSize);
      hash.push<3>(r.mVecSize);
      hash.push<3>(r.mColumns);
      hash.push<32>(r.mArraySize);
      for (auto const& m : r.mMembers) {
        pushMember(m);
      }
    };

  for (auto const& r : setResources) {
    // TODO: Are those bit sizes too much / sufficient?
    hash.push<7>(r.mStages);
    hash.push<20>(r.mAccess);
    hash.push<4>(r.mResourceType);
    hash.push<4>(r.mBaseType);
    hash.push<10>(r.mSet);
    hash.push<10>(r.mBinding);
    hash.push<10>(r.mLocation);
    hash.push<10>(r.mInputAttachmentIndex);
    hash.push<3>(r.mVecSize);
    hash.push<3>(r.mColumns);
    hash.push<32>(r.mArraySize);
    hash.push<32>(r.mOffset);
    hash.push<32>(r.mSize);

    for (auto const& m : r.mMembers) {
      pushMember(m);
    }
  }

  {
    std::unique_lock<std::mutex> lock(mMutex);
    auto                         cached = mCache.find(hash);
    if (cached != mCache.end()) { return cached->second; }
  }

  auto pool = std::make_shared<DescriptorPool>(mContext, setResources, set);

  std::unique_lock<std::mutex> lock(mMutex);
  mCache[hash] = pool;

  return pool;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DescriptorPoolCache::clear() {
  std::unique_lock<std::mutex> lock(mMutex);
  mCache.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
