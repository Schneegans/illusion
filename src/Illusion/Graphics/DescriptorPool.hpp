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

// ---------------------------------------------------------------------------------------- includes
#include "PipelineResource.hpp"

#include <list>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorPool {
 public:
  DescriptorPool(
    std::shared_ptr<Context> const&      context,
    std::vector<PipelineResource> const& setResources,
    uint32_t                             set);
  virtual ~DescriptorPool();

  std::shared_ptr<DescriptorSet> allocateDescriptorSet();

  std::shared_ptr<vk::DescriptorSetLayout> const& getLayout() const;

 private:
  const uint32_t                           mMaxSetsPerPool = 64;
  std::shared_ptr<Context>                 mContext;
  std::shared_ptr<vk::DescriptorSetLayout> mDescriptorSetLayout;
  std::vector<vk::DescriptorPoolSize>      mPoolSizes;
  uint32_t                                 mSet;

  // stores all descriptor pools and the number of descriptor sets
  // which have been allocated from those pools
  struct PoolInfo {
    std::shared_ptr<vk::DescriptorPool> mPool;
    uint32_t                            mAllocationCount = 0;
  };

  std::vector<std::shared_ptr<PoolInfo>> mDescriptorPools;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_POOL_HPP
