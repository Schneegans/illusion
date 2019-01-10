////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_VULKAN_PTR_HPP
#define ILLUSION_GRAPHICS_VULKAN_PTR_HPP

#include <functional>
#include <memory>

namespace Illusion::Graphics::VulkanPtr {

// shared_ptr based memory management for vulkan objects
template <typename T>
struct Identity {
  typedef T type;
};

template <typename T>
std::shared_ptr<T> create(
    T const& vkObject, typename Identity<std::function<void(T* obj)>>::type deleter) {
  return std::shared_ptr<T>(new T{vkObject}, deleter);
}

} // namespace Illusion::Graphics::VulkanPtr

#endif // ILLUSION_GRAPHICS_VULKAN_PTR_HPP
