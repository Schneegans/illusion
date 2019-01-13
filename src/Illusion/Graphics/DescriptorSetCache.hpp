////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DESCRIPTOR_SET_CACHE_HPP
#define ILLUSION_GRAPHICS_DESCRIPTOR_SET_CACHE_HPP

#include "../Core/BitHash.hpp"
#include "DescriptorSetReflection.hpp"

#include <map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The DescriptorSetCache can be used to avoid frequent recreation of similar DescriptorSets. It  //
// also simplifies the vk::DescriptorSet management if multiple pipelines use the same            //
// DescriptorSetLayouts. It is used by the CommandBuffer class.                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorSetCache {
 public:
  DescriptorSetCache(DevicePtr const& device);
  virtual ~DescriptorSetCache();

  // A reference to the acquired vk::DescriptorSet is also stored in the internal cache of this
  // object. Therefore it will not be deleted, even if the returned handle goes out of scope. A hash
  // based on the reflection will be used to store the handle.
  vk::DescriptorSetPtr acquireHandle(DescriptorSetReflectionPtr const& reflection);

  // This should only be used with handles created by the method above. The passed in handle is
  // marked as not being used anymore and will be returned by subsequent calls to acquireHandle()
  // if the construction parameters are the same. This will not delete the allocated
  // vk::DescriptorSet.
  void releaseHandle(vk::DescriptorSetPtr const& handle);

  // Calls releaseHandle() for all DescriptorSets which have been created by this
  // DescriptorSetCache.
  void releaseAll();

  // Clears all references to DescriptorSets created by this DescriptorSetCache. This will cause the
  // deletion of the all cached DescriptorSets if there are no other references around.
  void deleteAll();

 private:
  struct CacheEntry {
    DescriptorPoolPtr              mPool;
    std::set<vk::DescriptorSetPtr> mUsedHandels;
    std::set<vk::DescriptorSetPtr> mFreeHandels;
  };

  DevicePtr                                   mDevice;
  mutable std::map<Core::BitHash, CacheEntry> mCache;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_SET_CACHE_HPP
