////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SWAPCHAIN_HPP
#define ILLUSION_GRAPHICS_SWAPCHAIN_HPP

#include "RenderPass.hpp"

#include <functional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Swapchain {

 public:
  template <typename... Args>
  static SwapchainPtr create(Args&&... args) {
    return std::make_shared<Swapchain>(args...);
  };

  Swapchain(DevicePtr const& device, vk::SurfaceKHRPtr const& surface);
  virtual ~Swapchain();

  void              setEnableVsync(bool enable);
  void              markDirty();
  glm::uvec2 const& getExtent() const;

  void present(BackedImagePtr const& image, vk::SemaphorePtr const& renderFinishedSemaphore,
    vk::FencePtr const& signalFence);

 private:
  void chooseExtent();
  void chooseFormat();
  void createSwapchain();
  void createSemaphores();
  void createCommandBuffers();

  DevicePtr            mDevice;
  vk::SurfaceKHRPtr    mSurface;
  glm::uvec2           mExtent;
  vk::SurfaceFormatKHR mFormat;
  vk::SwapchainKHRPtr  mSwapchain;

  std::vector<vk::Image> mImages;
  uint32_t               mCurrentImageIndex = 0;

  std::vector<vk::SemaphorePtr> mImageAvailableSemaphores;
  std::vector<vk::SemaphorePtr> mCopyFinishedSemaphores;
  std::vector<CommandBufferPtr> mPresentCommandBuffers;
  uint32_t                      mCurrentPresentIndex = 0;

  bool mEnableVsync = true;
  bool mDirty       = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SWAPCHAIN_HPP
