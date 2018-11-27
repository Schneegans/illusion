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

  int getGraphicsFamily() const { return mGraphicsFamily; }
  int getComputeFamily() const { return mComputeFamily; }
  int getPresentFamily() const { return mPresentFamily; }

  void printInfo();

 private:
  int chooseQueueFamily(vk::QueueFlagBits caps) const;
  int choosePresentQueueFamily(vk::Instance const& instance) const;

  int mGraphicsFamily = -1, mComputeFamily = -1, mPresentFamily = -1;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PHYSICAL_DEVICE_HPP
