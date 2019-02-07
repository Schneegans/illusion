////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP
#define ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP

#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The PhysicalDevice is a representation of a real hardware device of your system. You can use   //
// it to query information on the capabilites of your GPU. You will need a PhysicalDevice so that //
// you can create the actual Device which you will need to create Vulkan resources.               //
////////////////////////////////////////////////////////////////////////////////////////////////////

class PhysicalDevice : public vk::PhysicalDevice, public Core::StaticCreate<PhysicalDevice> {
 public:
  // The PysicalDeviceis created by the Instance. So you do not have to create this on your own.
  PhysicalDevice(vk::Instance const& instance, vk::PhysicalDevice const& device);

  // Tries to find a memory type matching both parameters. This will throw a std::runtime_error when
  // there is no suitable memory type.
  uint32_t findMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const;

  // The PhysicalDevice will try to pick different Queues for each QueueType. If that is not
  // possible, it might happen that two or all three QueueTypes actually refer to the same queue.
  uint32_t getQueueFamily(QueueType type) const;
  uint32_t getQueueIndex(QueueType type) const;

  // Prints a complete list of your hardware capabilities to std::cout.
  void printInfo();

 private:
  std::array<uint32_t, 3> mQueueFamilies = {0, 0, 0};
  std::array<uint32_t, 3> mQueueIndices  = {0, 0, 0};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP
