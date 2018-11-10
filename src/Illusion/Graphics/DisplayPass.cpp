////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "DisplayPass.hpp"

#include "../Core/Logger.hpp"
#include "PhysicalDevice.hpp"
#include "Window.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

DisplayPass::DisplayPass(
  std::shared_ptr<Context> const& context, std::shared_ptr<vk::SurfaceKHR> const& surface)
  : RenderPass(context)
  , mSurface(surface) {

  mSwapchainSemaphore = createSwapchainSemaphore();
  mWaitSemaphores.push_back(mSwapchainSemaphore);

  ILLUSION_TRACE << "Creating DisplayPass." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DisplayPass::~DisplayPass() {
  // mSwapchainSemaphore.reset();
  // mSwapchain.reset();
  // mSurface.reset();
  ILLUSION_TRACE << "Deleting DisplayPass." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayPass::setEnableVsync(bool enable) {
  if (enable != mEnableVsync) {
    mEnableVsync = enable;
    markSwapChainDirty();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayPass::render() {
  if (mSwapchainDirty) {
    mContext->getDevice()->waitIdle();

    // delete old one first
    mSwapchain.reset();

    // then create new one
    setExtent(chooseExtent());
    setRingBufferSize(chooseSwapchainImageCount());

    mSwapchainFormat = chooseSwapchainFormat();
    mSwapchain       = createSwapChain();

    auto                   tmp = mContext->getDevice()->getSwapchainImagesKHR(*mSwapchain);
    std::vector<vk::Image> swapchainImages;

    for (uint32_t i{0}; i < getRingBufferSize(); ++i) {
      swapchainImages.push_back(tmp[i]);
    }

    setSwapchainInfo(swapchainImages, mSwapchainFormat.format);

    mSwapchainDirty = false;
  }

  auto result = mContext->getDevice()->acquireNextImageKHR(
    *mSwapchain,
    std::numeric_limits<uint64_t>::max(),
    *mSwapchainSemaphore,
    nullptr,
    &mCurrentRingBufferIndex);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    // mark dirty and call this method again
    mSwapchainDirty = true;
    render();
    return;
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    ILLUSION_ERROR << "Suboptimal swap chain!" << std::endl;
  }

  RenderPass::render();

  // present on mOutputWindow ----------------------------------------------------------------------

  vk::SwapchainKHR swapChains[]     = {*mSwapchain};
  vk::Semaphore    waitSemaphores[] = {*mSignalSemaphore};

  vk::PresentInfoKHR presentInfo;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = waitSemaphores;
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = swapChains;
  presentInfo.pImageIndices      = &mCurrentRingBufferIndex;

  result = mContext->getPresentQueue().presentKHR(presentInfo);
  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
    // when does this happen?
    ILLUSION_ERROR << "out of date 3!" << std::endl;
  } else if (result != vk::Result::eSuccess) {
    // when does this happen?
    ILLUSION_ERROR << "out of date 4!" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Semaphore> DisplayPass::createSwapchainSemaphore() const {
  vk::SemaphoreCreateInfo info;
  return mContext->createSemaphore(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Extent2D DisplayPass::chooseExtent() const {
  auto capabilities = mContext->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*mSurface);

  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {

    // when does this happen?
    ILLUSION_WARNING << "TODO" << std::endl;
    vk::Extent2D actualExtent = {500, 500};

    actualExtent.width = std::max(
      capabilities.minImageExtent.width,
      std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
      capabilities.minImageExtent.height,
      std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SurfaceFormatKHR DisplayPass::chooseSwapchainFormat() const {
  auto formats = mContext->getPhysicalDevice()->getSurfaceFormatsKHR(*mSurface);

  if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  for (const auto& format : formats) {
    if (
      format.format == vk::Format::eB8G8R8A8Unorm &&
      format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return format;
    }
  }

  return formats[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t DisplayPass::chooseSwapchainImageCount() const {
  auto capabilities = mContext->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*mSurface);

  uint32_t imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  return imageCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::SwapchainKHR> DisplayPass::createSwapChain() const {

  auto capabilities = mContext->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*mSurface);
  auto formats      = mContext->getPhysicalDevice()->getSurfaceFormatsKHR(*mSurface);
  auto presentModes = mContext->getPhysicalDevice()->getSurfacePresentModesKHR(*mSurface);

  // Fifo is actually required to be supported and is a decent choice for V-Sync
  vk::PresentModeKHR presentMode{vk::PresentModeKHR::eFifo};

  if (!mEnableVsync) {
    // Immediate is an option for no V-Sync but will result in tearing
    for (auto mode : presentModes) {
      if (mode == vk::PresentModeKHR::eImmediate) {
        presentMode = mode;
        break;
      }
    }

    // mailbox mode would be better
    for (auto mode : presentModes) {
      if (mode == vk::PresentModeKHR::eMailbox) {
        presentMode = mode;
        break;
      }
    }
  }

  vk::SwapchainCreateInfoKHR info;
  info.surface          = *mSurface;
  info.minImageCount    = getRingBufferSize();
  info.imageFormat      = mSwapchainFormat.format;
  info.imageColorSpace  = mSwapchainFormat.colorSpace;
  info.imageExtent      = getExtent();
  info.imageArrayLayers = 1;
  info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
  info.preTransform     = capabilities.currentTransform;
  info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  info.presentMode      = presentMode;
  info.clipped          = true;
  info.oldSwapchain     = nullptr; // this could be optimized

  uint32_t graphicsFamily = mContext->getPhysicalDevice()->getGraphicsFamily();
  uint32_t presentFamily  = mContext->getPhysicalDevice()->getPresentFamily();

  // this check should not be neccessary, but the validation layers complain
  // when only glfwGetPhysicalDevicePresentationSupport was used to check for
  // presentation support
  if (!mContext->getPhysicalDevice()->getSurfaceSupportKHR(presentFamily, *mSurface)) {
    ILLUSION_ERROR << "The selected queue family does not "
                   << "support presentation!" << std::endl;
  }

  if (graphicsFamily != presentFamily) {
    uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};
    info.imageSharingMode         = vk::SharingMode::eConcurrent;
    info.queueFamilyIndexCount    = 2;
    info.pQueueFamilyIndices      = queueFamilyIndices;
  } else {
    info.imageSharingMode      = vk::SharingMode::eExclusive;
    info.queueFamilyIndexCount = 0;       // Optional
    info.pQueueFamilyIndices   = nullptr; // Optional
  }

  return mContext->createSwapChainKhr(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
