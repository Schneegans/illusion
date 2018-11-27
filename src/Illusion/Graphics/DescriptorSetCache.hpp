////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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
// also simplifies the DescriptorSet management if multiple pipelines use the same                //
// DescriptorSetLayouts. It is a good idea to use an instance of this class as part of your frame //
// resources in a ring buffer fashion.                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorSetCache {
 public:
  DescriptorSetCache(std::shared_ptr<Device> const& device);
  virtual ~DescriptorSetCache();

  // A reference to the acquired DescriptorSet is also stored in the internal cache of this object.
  // Therefore it will not be deleted, even if the returned handle goes out of scope. A hash based
  // on the reflection will be used to store the handle.
  std::shared_ptr<DescriptorSet> acquireHandle(
    std::shared_ptr<DescriptorSetReflection> const& reflection);

  // This should only be used with handles created by the method above. The passed in handle is
  // marked as not being used anymore and will be returned by subsequent calls to acquireHandle()
  // if the construction parameters are the same. This will not delete the allocated DescriptorSet.
  void releaseHandle(std::shared_ptr<DescriptorSet> const& handle);

  // Calls releaseHandle() for all DescriptoSets which have been created by this DescriptorSetCache.
  void releaseAll();

  // Clears all references to DescriptoSets created by this DescriptorSetCache. This will most
  // likely cause the deletion of the all cached DescriptoSets.
  void deleteAll();

 private:
  struct CacheEntry {
    std::shared_ptr<DescriptorPool>          mPool;
    std::set<std::shared_ptr<DescriptorSet>> mUsedHandels;
    std::set<std::shared_ptr<DescriptorSet>> mFreeHandels;
  };

  std::shared_ptr<Device>                     mDevice;
  mutable std::map<Core::BitHash, CacheEntry> mCache;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_SET_CACHE_HPP
