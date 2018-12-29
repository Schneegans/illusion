////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DESCRIPTOR_POOL_HPP
#define ILLUSION_GRAPHICS_DESCRIPTOR_POOL_HPP

#include "fwd.hpp"

#include <list>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The DescriptorPool is used by the DescriptorSetCache to create the DescriptorSets. For a given //
// DescriptorSetReflection it is able to create an arbitrary amount of DescriptorSets.            //
// Internally, vk::DescriptorPools will be allocated on demand, whenever the maximum number of    //
// vk::DescriptorSet allocations is reached.                                                      //
// Reference counting on the returned handle is used to decide when a Descriptor set can be freed //
// and returned to the allocating vk::DescriporPool.                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorPool {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static DescriptorPoolPtr create(Args&&... args) {
    return std::make_shared<DescriptorPool>(args...);
  };

  DescriptorPool(DevicePtr const& device, DescriptorSetReflectionPtr const& reflection);
  virtual ~DescriptorPool();

  // Allocates a fresh vk::DescriptorSet, may create a vk::DescriptorPool if no free pool is
  // available. Once the reference count on this handle runs out of scope, the vk::DescriptorSet
  // will be freed.
  vk::DescriptorSetPtr allocateDescriptorSet();

 private:
  const uint32_t                      mMaxSetsPerPool = 64;
  DevicePtr                           mDevice;
  DescriptorSetReflectionPtr          mReflection;
  std::vector<vk::DescriptorPoolSize> mPoolSizes;

  // The cache stores all vk::DescriptorPools and the number of descriptor sets which have been
  // allocated from those pools.
  struct PoolInfo {
    vk::DescriptorPoolPtr mPool;
    uint32_t              mAllocationCount = 0;
  };

  std::vector<std::shared_ptr<PoolInfo>> mDescriptorPools;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_POOL_HPP
