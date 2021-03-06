////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DescriptorPool.hpp"

#include "../Core/Logger.hpp"
#include "DescriptorSetReflection.hpp"
#include "Device.hpp"
#include "Utils.hpp"
#include "VulkanPtr.hpp"

#include <iostream>
#include <unordered_map>
#include <utility>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

// Map PipelineResource::ResourceType to vk::DescriptorType.
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

DescriptorPool::DescriptorPool(std::string const& name, DeviceConstPtr device,
    DescriptorSetReflectionConstPtr const& reflection)
    : Core::NamedObject(name)
    , mDevice(std::move(device))
    , mReflection(reflection) {

  // Get the number of descriptors for each DescriptorType.
  std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCounts;
  for (auto const& r : reflection->getResources()) {
    descriptorTypeCounts[resourceTypeMapping.at(r.second.mResourceType)] += r.second.mArraySize;
  }

  // Now multiply those numbers with the number of descriptor sets allocated from each internal
  // vk::DescriptorPool. This is required for the lazy pool creation.
  for (auto it : descriptorTypeCounts) {
    vk::DescriptorPoolSize pool;
    pool.type            = it.first;
    pool.descriptorCount = static_cast<uint32_t>(it.second * mMaxSetsPerPool);
    mPoolSizes.push_back(pool);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DescriptorPool::~DescriptorPool() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorSetPtr DescriptorPool::allocateDescriptorSet() {

  // Throw error when there is no resource in this descriptor set.
  if (mPoolSizes.empty()) {
    throw std::runtime_error(
        "Cannot allocated DescriptorSet: Set does not contain any active resources!");
  }

  // Find a free pool.
  std::shared_ptr<PoolInfo> pool;

  for (auto& p : mDescriptorPools) {
    if (p->mAllocationCount + 1 < mMaxSetsPerPool) {
      pool = p;
      p->mAllocationCount += 1;
      break;
    }
  }

  // If no free pool has been found, create a new one.
  if (!pool) {
    vk::DescriptorPoolCreateInfo info;
    info.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
    info.pPoolSizes    = mPoolSizes.data();
    info.maxSets       = mMaxSetsPerPool;
    info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    pool                   = std::make_shared<PoolInfo>();
    pool->mPool            = mDevice->createDescriptorPool(getName(), info);
    pool->mAllocationCount = 0;
    mDescriptorPools.push_back(pool);
  }

  // Now allocate the descriptor set from this pool.
  vk::DescriptorSetLayout descriptorSetLayout = *mReflection->getLayout();

  vk::DescriptorSetAllocateInfo info;
  info.descriptorPool     = *pool->mPool;
  info.descriptorSetCount = 1;
  info.pSetLayouts        = &descriptorSetLayout;

  Core::Logger::traceCreation("vk::DescriptorSet", "DescriptorSet from " + getName());

  auto device = mDevice->getHandle();
  auto name   = getName();
  return VulkanPtr::create(
      device->allocateDescriptorSets(info)[0], [device, pool, name](vk::DescriptorSet* obj) {
        Core::Logger::traceDeletion("vk::DescriptorSet", "DescriptorSet from " + name);
        --pool->mAllocationCount;
        device->freeDescriptorSets(*pool->mPool, *obj);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
