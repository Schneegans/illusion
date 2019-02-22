////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SWAPCHAIN_HPP
#define ILLUSION_GRAPHICS_SWAPCHAIN_HPP

#include "RenderPass.hpp"

#include <functional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This class is primarily as member of the Window class. It manages presentation of images on    //
// the window's surface. The image given to the present() method will be blitted to the current   //
// swapchain image.                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Swapchain : public Core::StaticCreate<Swapchain>, public Core::NamedObject {
 public:
  // This is called by the Window.
  Swapchain(std::string const& name, DeviceConstPtr device, vk::SurfaceKHRPtr surface);
  virtual ~Swapchain();

  // For now, vk::PresentModeKHR::eFifo is used for v-sync, for no v-sync either
  // vk::PresentModeKHR::eImmediate (if supported) or preferably vk::PresentModeKHR::eMailbox (if
  // supported) are used.
  void setEnableVsync(bool enable);

  // This will trigger a re-creation of the SwapChain. This is called by the Window on size changes.
  void markDirty();

  // Get the current size of the swapchain images.
  glm::uvec2 const& getExtent() const;

  // Blits the given image to one of the swapchain images. The operation will wait for the given
  // semaphore and will signal the given fence once it finishes.
  void present(BackedImagePtr const& image, vk::SemaphorePtr const& renderFinishedSemaphore,
      vk::FencePtr const& signalFence);

 private:
  void recreate();

  DeviceConstPtr       mDevice;
  vk::SurfaceKHRPtr    mSurface;
  glm::uvec2           mExtent{};
  vk::SurfaceFormatKHR mFormat{};
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
