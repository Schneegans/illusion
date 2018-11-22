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
#include "DescriptorPool.hpp"

#include "../Core/Logger.hpp"
#include "Context.hpp"
#include "DescriptorSet.hpp"
#include "SetResources.hpp"
#include "Utils.hpp"

#include <iostream>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorPool::DescriptorPool(
  std::shared_ptr<Context> const&                 context,
  vk::DescriptorSetLayoutCreateInfo const&        info,
  std::shared_ptr<vk::DescriptorSetLayout> const& layout,
  uint32_t                                        set)
  : mContext(context)
  , mDescriptorSetLayout(layout)
  , mSet(set) {

  ILLUSION_TRACE << "Creating DescriptorPool." << std::endl;

  // calculate pool sizes for later pool creation --------------------------------------------------
  std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCounts;
  for (uint32_t i(0); i < info.bindingCount; ++i) {
    descriptorTypeCounts[info.pBindings[i].descriptorType] += info.pBindings[i].descriptorCount;
  }

  for (auto it : descriptorTypeCounts) {
    vk::DescriptorPoolSize pool;
    pool.type            = it.first;
    pool.descriptorCount = static_cast<uint32_t>(it.second * mMaxSetsPerPool);
    mPoolSizes.push_back(pool);
  }
} // namespace Illusion::Graphics

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorPool::~DescriptorPool() { ILLUSION_TRACE << "Deleting DescriptorPool." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<DescriptorSet> DescriptorPool::allocateDescriptorSet() {

  if (mPoolSizes.size() == 0) {
    throw std::runtime_error(
      "Cannot allocated DescriptorSet: Set does not contain any active resources!");
  }

  // find a free pool
  std::shared_ptr<PoolInfo> pool;

  for (auto& p : mDescriptorPools) {
    if (p->mAllocationCount + 1 < mMaxSetsPerPool) {
      pool = p;
      p->mAllocationCount += 1;
      break;
    }
  }

  // if no free pool has been found, create a new one
  if (!pool) {
    vk::DescriptorPoolCreateInfo info;
    info.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
    info.pPoolSizes    = mPoolSizes.data();
    info.maxSets       = mMaxSetsPerPool;
    info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    pool                   = std::make_shared<PoolInfo>();
    pool->mPool            = mContext->createDescriptorPool(info);
    pool->mAllocationCount = 0;
    mDescriptorPools.push_back(pool);
  }

  // now allocate the descriptor set from this pool
  vk::DescriptorSetLayout descriptorSetLayouts[] = {*mDescriptorSetLayout};

  vk::DescriptorSetAllocateInfo info;
  info.descriptorPool     = *pool->mPool;
  info.descriptorSetCount = 1;
  info.pSetLayouts        = descriptorSetLayouts;

  ILLUSION_TRACE << "Allocating DescriptorSet." << std::endl;

  auto device{mContext->getDevice()};
  return std::shared_ptr<DescriptorSet>(
    new DescriptorSet(mContext, mSet, device->allocateDescriptorSets(info)[0]),
    [device, pool](DescriptorSet* obj) {
      ILLUSION_TRACE << "Freeing DescriptorSet." << std::endl;
      --pool->mAllocationCount;
      device->waitIdle();
      device->freeDescriptorSets(*pool->mPool, *obj);
      delete obj;
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
