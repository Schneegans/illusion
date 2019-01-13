////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP
#define ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP

#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class PhysicalDevice : public vk::PhysicalDevice {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static PhysicalDevicePtr create(Args&&... args) {
    return std::make_shared<PhysicalDevice>(args...);
  };

  PhysicalDevice(vk::Instance const& instance, vk::PhysicalDevice const& device);

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

  // queues of this family can do graphics, compute, transfer and presentation
  uint32_t getQueueFamily(QueueType type) const;
  uint32_t getQueueIndex(QueueType type) const;

  void printInfo();

 private:
  std::array<uint32_t, 3> mQueueFamilies = {0, 0, 0};
  std::array<uint32_t, 3> mQueueIndices  = {0, 0, 0};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP
