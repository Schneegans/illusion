////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DescriptorPool.hpp"

#include "../Core/Logger.hpp"
#include "DescriptorSetReflection.hpp"
#include "Device.hpp"
#include "Utils.hpp"

#include <iostream>
#include <unordered_map>

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

DescriptorPool::DescriptorPool(
    DevicePtr const& device, DescriptorSetReflectionPtr const& reflection)
    : mDevice(device)
    , mReflection(reflection) {

  ILLUSION_TRACE << "Creating DescriptorPool." << std::endl;

  // calculate pool sizes for later pool creation
  std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCounts;
  for (auto const& r : reflection->getResources()) {
    descriptorTypeCounts[resourceTypeMapping.at(r.second.mResourceType)] += r.second.mArraySize;
  }

  for (auto it : descriptorTypeCounts) {
    vk::DescriptorPoolSize pool;
    pool.type            = it.first;
    pool.descriptorCount = static_cast<uint32_t>(it.second * mMaxSetsPerPool);
    mPoolSizes.push_back(pool);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorPool::~DescriptorPool() {
  ILLUSION_TRACE << "Deleting DescriptorPool." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorSetPtr DescriptorPool::allocateDescriptorSet() {

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
    pool->mPool            = mDevice->createDescriptorPool(info);
    pool->mAllocationCount = 0;
    mDescriptorPools.push_back(pool);
  }

  // now allocate the descriptor set from this pool
  vk::DescriptorSetLayout descriptorSetLayouts[] = {*mReflection->getLayout()};

  vk::DescriptorSetAllocateInfo info;
  info.descriptorPool     = *pool->mPool;
  info.descriptorSetCount = 1;
  info.pSetLayouts        = descriptorSetLayouts;

  ILLUSION_TRACE << "Allocating DescriptorSet." << std::endl;

  auto device{mDevice->getHandle()};
  return Utils::makeVulkanPtr(
      device->allocateDescriptorSets(info)[0], [device, pool](vk::DescriptorSet* obj) {
        ILLUSION_TRACE << "Freeing DescriptorSet." << std::endl;
        --pool->mAllocationCount;
        device->freeDescriptorSets(*pool->mPool, *obj);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
