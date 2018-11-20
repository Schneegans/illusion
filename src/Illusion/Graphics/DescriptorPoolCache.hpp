////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DESCRIPTOR_POOL_CACHE_HPP
#define ILLUSION_GRAPHICS_DESCRIPTOR_POOL_CACHE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Core/BitHash.hpp"
#include "PipelineResource.hpp"

#include <map>
#include <mutex>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorPoolCache {
 public:
  DescriptorPoolCache(std::shared_ptr<Context> const& context);
  virtual ~DescriptorPoolCache();

  std::shared_ptr<DescriptorPool> get(
    std::vector<PipelineResource> const& setResources, uint32_t set);

  void clear();

 private:
  std::shared_ptr<Context>                                 mContext;
  std::mutex                                               mMutex;
  std::map<Core::BitHash, std::shared_ptr<DescriptorPool>> mCache;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_POOL_CACHE_HPP
