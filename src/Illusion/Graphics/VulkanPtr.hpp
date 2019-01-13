////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_VULKAN_PTR_HPP
#define ILLUSION_GRAPHICS_VULKAN_PTR_HPP

#include <functional>
#include <memory>

namespace Illusion::Graphics::VulkanPtr {

////////////////////////////////////////////////////////////////////////////////////////////////////
// In Illusion, all Vulkan resources which are explicitly created by some other object            //
// ("children") and need to be destroyed by this other object ("parent") later, are wrapped in a  //
// std::shader_ptr. This std::shared_ptr stores a special deleter lambda which captures a         //
// reference to the "parent" which is responsible for the destruction. This ensures that all      //
// "children" will be deleted before the destructor of the "parent" is called.                    //
// The Device class makes extenive use of this pattern, but it is also used in other classes.     //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Helper template
template <typename T>
struct Identity {
  typedef T type;
};

// The function takes two arguments. The first is the Vulkan object to be wrapped, the second is the
// deleter lambda which should capture a std::shared_ptr to the object which created the wrapped
// object.
// Here is an example how we could create a vk::Image, device is a std::shared_ptr to a vk::Device.
//
// vk::ImageCreateInfo info;
// auto ptr = VulkanPtr::create(device->createImage(info), [device](vk::Image* obj) {
//   device->destroyImage(*obj);
//   delete obj;
// });
//
template <typename T>
std::shared_ptr<T> create(
    T const& vkObject, typename Identity<std::function<void(T* obj)>>::type deleter) {
  return std::shared_ptr<T>(new T{vkObject}, deleter);
}

} // namespace Illusion::Graphics::VulkanPtr

#endif // ILLUSION_GRAPHICS_VULKAN_PTR_HPP
