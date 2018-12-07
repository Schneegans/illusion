////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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
  PhysicalDevice(vk::Instance const& instance, vk::PhysicalDevice const& device);

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

  // queues of this family can do graphics, compute, transfer and presentation
  int32_t  getQueueFamily(QueueType type) const;
  uint32_t getQueueIndex(QueueType type) const;

  void printInfo();

 private:
  std::array<int32_t, 3>  mQueueFamilies = {-1, -1, -1};
  std::array<uint32_t, 3> mQueueIndices  = {0, 0, 0};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP
