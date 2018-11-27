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

#include "../Core/BitHash.hpp"
#include "fwd.hpp"

#include <map>
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The BufferCache can be used to avoid frequent recreation of similar buffers. For example, this //
// is quite useful for uniform buffer allocation. It is a good idea to use an instance of this    //
// class as part of your frame resources in a ring buffer fashion.                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class BufferCache {
 public:
  BufferCache(std::shared_ptr<Device> const& device);
  virtual ~BufferCache();

  // A reference to the acquired buffer is also stored in the internal cache of this object.
  // Therefore it will not be deleted, even if the returned handle goes out of scope. A hash based
  // on the given parameters will be used to store the handle.
  std::shared_ptr<BackedBuffer> acquireHandle(
    vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

  // This should only be used with handles created by the method above. The passed in handle is
  // marked as not being used anymore and will be returned by subsequent calls to acquireHandle()
  // if the construction parameters are the same. This will not delete the allocated buffer.
  void releaseHandle(std::shared_ptr<BackedBuffer> const& handle);

  // Calls releaseHandle() for all buffers which have been created by this BufferCache.
  void releaseAll();

  // Clears all references to buffers created by this BufferCache. This will most likely cause the
  // deletion of the all cached buffers.
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
