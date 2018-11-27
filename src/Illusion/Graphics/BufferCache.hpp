////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_BUFFER_CACHE_HPP
#define ILLUSION_GRAPHICS_BUFFER_CACHE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Core/BitHash.hpp"
#include "fwd.hpp"

#include <map>
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class BufferCache {
 public:
  BufferCache(std::shared_ptr<Device> const& device);
  virtual ~BufferCache();

  std::shared_ptr<BackedBuffer> acquireHandle(
    vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
  void releaseHandle(std::shared_ptr<BackedBuffer> const& handle);

  void releaseAll();
  void deleteAll();

 private:
  struct CacheEntry {
    std::set<std::shared_ptr<BackedBuffer>> mUsedHandels;
    std::set<std::shared_ptr<BackedBuffer>> mFreeHandels;
  };

  std::shared_ptr<Device>                     mDevice;
  mutable std::map<Core::BitHash, CacheEntry> mCache;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_BUFFER_CACHE_HPP
