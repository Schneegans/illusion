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

// ---------------------------------------------------------------------------------------- includes
#include "../Core/BitHash.hpp"
#include "DescriptorSetReflection.hpp"

#include <list>
#include <map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorSetCache {
 public:
  DescriptorSetCache(std::shared_ptr<Context> const& context);
  virtual ~DescriptorSetCache();

  std::shared_ptr<DescriptorSet> acquireHandle(
    std::shared_ptr<DescriptorSetReflection> const& reflection);

  void releaseHandle(std::shared_ptr<DescriptorSet> const& handle);

  void releaseAll();
  void deleteAll();

 private:
  struct CacheEntry {
    std::shared_ptr<DescriptorPool>          mPool;
    std::set<std::shared_ptr<DescriptorSet>> mUsedHandels;
    std::set<std::shared_ptr<DescriptorSet>> mFreeHandels;
  };

  std::shared_ptr<Context>                    mContext;
  mutable std::map<Core::BitHash, CacheEntry> mCache;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_SET_CACHE_HPP
