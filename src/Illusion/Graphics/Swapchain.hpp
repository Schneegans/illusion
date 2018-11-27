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

// ---------------------------------------------------------------------------------------- includes
#include "RenderPass.hpp"

#include <functional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Swapchain {

 public:
  // -------------------------------------------------------------------------------- public methods
  Swapchain(std::shared_ptr<Device> const& device, std::shared_ptr<vk::SurfaceKHR> const& surface);
  virtual ~Swapchain();

  void              setEnableVsync(bool enable);
  void              markDirty();
  glm::uvec2 const& getExtent() const;

  void present(
    std::shared_ptr<BackedImage> const&   image,
    std::shared_ptr<vk::Semaphore> const& renderFinishedSemaphore,
    std::shared_ptr<vk::Fence> const&     signalFence);

 private:
  // ------------------------------------------------------------------------------- private methods
  void chooseExtent();
  void chooseFormat();
  void createSwapchain();
  void createSemaphores();
  void createCommandBuffers();

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Device>           mDevice;
  std::shared_ptr<vk::SurfaceKHR>   mSurface;
  glm::uvec2                        mExtent;
  vk::SurfaceFormatKHR              mFormat;
  std::shared_ptr<vk::SwapchainKHR> mSwapchain;

  std::vector<vk::Image> mImages;
  uint32_t               mCurrentImageIndex = 0;

  std::vector<std::shared_ptr<vk::Semaphore>> mImageAvailableSemaphores;
  std::vector<std::shared_ptr<vk::Semaphore>> mCopyFinishedSemaphores;
  std::vector<std::shared_ptr<CommandBuffer>> mPresentCommandBuffers;
  uint32_t                                    mCurrentPresentIndex = 0;

  bool mEnableVsync = true;
  bool mDirty       = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SWAPCHAIN_HPP
