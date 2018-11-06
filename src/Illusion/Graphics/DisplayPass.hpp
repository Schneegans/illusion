////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DISPLAY_PASS_HPP
#define ILLUSION_GRAPHICS_DISPLAY_PASS_HPP

// ---------------------------------------------------------------------------------------- includes
#include "RenderPass.hpp"

#include <functional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class DisplayPass : public RenderPass {

 public:
  // -------------------------------------------------------------------------------- public methods
  DisplayPass(
    std::shared_ptr<Engine> const& engine, std::shared_ptr<vk::SurfaceKHR> const& surface);
  virtual ~DisplayPass();

  void         setEnableVsync(bool enable);
  void         markSwapChainDirty() { mSwapchainDirty = true; }
  virtual void render() override;

 private:
  // ------------------------------------------------------------------------------- private methods
  std::shared_ptr<vk::Semaphore>    createSwapchainSemaphore() const;
  vk::Extent2D                      chooseExtent() const;
  vk::SurfaceFormatKHR              chooseSwapchainFormat() const;
  uint32_t                          chooseSwapchainImageCount() const;
  std::shared_ptr<vk::SwapchainKHR> createSwapChain() const;

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<vk::SurfaceKHR> mSurface;

  std::shared_ptr<vk::Semaphore>    mSwapchainSemaphore;
  vk::SurfaceFormatKHR              mSwapchainFormat;
  std::shared_ptr<vk::SwapchainKHR> mSwapchain;

  bool mEnableVsync    = true;
  bool mSwapchainDirty = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DISPLAY_PASS_HPP
