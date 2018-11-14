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
#include "Utils.hpp"

#include <iostream>
#include <unordered_map>

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

DescriptorPool::DescriptorPool(
  std::shared_ptr<Context> const& context, std::vector<PipelineResource> const& setResources)
  : mContext(context) {

  ILLUSION_TRACE << "Creating DescriptorPool." << std::endl;

  // create descriptor set layout ------------------------------------------------------------------
  std::vector<vk::DescriptorSetLayoutBinding> bindings;

  for (auto const& r : setResources) {
    auto t = r.mResourceType;
    if (
      t != PipelineResource::ResourceType::eInput && t != PipelineResource::ResourceType::eOutput &&
      t != PipelineResource::ResourceType::ePushConstantBuffer) {

      bindings.push_back({r.mBinding, resourceTypeMapping.at(t), r.mArraySize, r.mStages});
    }
  }

  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  descriptorSetLayoutInfo.pBindings    = bindings.data();

  mDescriptorSetLayout = mContext->createDescriptorSetLayout(descriptorSetLayoutInfo);

  // calculate pool sizes for later pool creation --------------------------------------------------
  std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCounts;
  for (auto const& r : setResources) {
    auto t = r.mResourceType;
    if (
      t != PipelineResource::ResourceType::eInput && t != PipelineResource::ResourceType::eOutput &&
      t != PipelineResource::ResourceType::ePushConstantBuffer) {

      descriptorTypeCounts[resourceTypeMapping.at(t)] += r.mArraySize;
    }
  }

  for (auto it : descriptorTypeCounts) {
    vk::DescriptorPoolSize pool;
    pool.type            = it.first;
    pool.descriptorCount = static_cast<uint32_t>(it.second * mMaxSetsPerPool);
    mPoolSizes.push_back(pool);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorPool::~DescriptorPool() { ILLUSION_TRACE << "Deleting DescriptorPool." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorSet> DescriptorPool::allocateDescriptorSet() {

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

  ILLUSION_TRACE << "Creating vk::DescriptorSet." << std::endl;

  auto device{mContext->getDevice()};
  return Utils::makeVulkanPtr(
    device->allocateDescriptorSets(info)[0], [device, pool](vk::DescriptorSet* obj) {
      ILLUSION_TRACE << "Deleting vk::DescriptorSet." << std::endl;
      --pool->mAllocationCount;
      device->freeDescriptorSets(*pool->mPool, *obj);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::DescriptorSetLayout> const& DescriptorPool::getLayout() const {
  return mDescriptorSetLayout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
