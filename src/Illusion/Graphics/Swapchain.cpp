////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Swapchain.hpp"

#include "../Core/Logger.hpp"
#include "CommandBuffer.hpp"
#include "PhysicalDevice.hpp"
#include "Window.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

Swapchain::Swapchain(
  std::shared_ptr<Device> const& device, std::shared_ptr<vk::SurfaceKHR> const& surface)
  : mDevice(device)
  , mSurface(surface) {

  ILLUSION_TRACE << "Creating Swapchain." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Swapchain::~Swapchain() { ILLUSION_TRACE << "Deleting Swapchain." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::setEnableVsync(bool enable) {
  if (enable != mEnableVsync) {
    mEnableVsync = enable;
    markDirty();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::markDirty() { mDirty = true; }

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 const& Swapchain::getExtent() const { return mExtent; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::present(
  std::shared_ptr<BackedImage> const&   image,
  std::shared_ptr<vk::Semaphore> const& renderFinishedSemaphore,
  std::shared_ptr<vk::Fence> const&     signalFence) {

  if (mDirty) {
    mDevice->waitIdle();

    // delete old one first
    mSwapchain.reset();
    mImageAvailableSemaphores.clear();
    mCopyFinishedSemaphores.clear();
    mPresentCommandBuffers.clear();

    // then create new one
    chooseExtent();
    chooseFormat();
    createSwapchain();

    mImages = mDevice->getHandle()->getSwapchainImagesKHR(*mSwapchain);

    auto cmd = std::make_shared<CommandBuffer>(mDevice);
    cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    for (auto const& image : mImages) {
      cmd->transitionImageLayout(
        image, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
    }
    cmd->end();
    cmd->submit();
    cmd->waitIdle();

    createSemaphores();
    createCommandBuffers();

    mDirty = false;
  }

  mCurrentPresentIndex = (mCurrentPresentIndex + 1) % mImages.size();

  auto result = mDevice->getHandle()->acquireNextImageKHR(
    *mSwapchain,
    std::numeric_limits<uint64_t>::max(),
    *mImageAvailableSemaphores[mCurrentPresentIndex],
    nullptr,
    &mCurrentImageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    // mark dirty and call this method again
    mDirty = true;
    present(image, renderFinishedSemaphore, signalFence);
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    ILLUSION_ERROR << "Suboptimal swap chain!" << std::endl;
  }

  // copy image ------------------------------------------------------------------------------------
  {
    auto const& cmd = mPresentCommandBuffers[mCurrentPresentIndex];
    cmd->reset();
    cmd->begin();

    vk::ImageSubresourceLayers subResource;
    subResource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    subResource.baseArrayLayer = 0;
    subResource.mipLevel       = 0;
    subResource.layerCount     = 1;

    // if source image is multisampled, resolve it
    if (image->mInfo.samples != vk::SampleCountFlagBits::e1) {
      vk::ImageResolve region;
      region.srcSubresource = subResource;
      region.dstSubresource = subResource;
      region.srcOffset      = vk::Offset3D(0, 0, 0);
      region.dstOffset      = vk::Offset3D(0, 0, 0);
      region.extent.width   = image->mInfo.extent.width;
      region.extent.height  = image->mInfo.extent.height;
      region.extent.depth   = 1;

      cmd->resolveImage(
        *image->mImage,
        vk::ImageLayout::eTransferSrcOptimal,
        mImages[mCurrentImageIndex],
        vk::ImageLayout::eTransferDstOptimal,
        region);
    }
    // Else do an image blit.
    else {
      cmd->transitionImageLayout(
        *image->mImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
      cmd->transitionImageLayout(
        mImages[mCurrentImageIndex],
        vk::ImageLayout::ePresentSrcKHR,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
      vk::ImageBlit info;
      cmd->blitImage(
        *image->mImage,
        mImages[mCurrentImageIndex],
        {image->mInfo.extent.width, image->mInfo.extent.height},
        mExtent,
        vk::Filter::eNearest);
      cmd->transitionImageLayout(
        *image->mImage,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::PipelineStageFlagBits::eTransfer);
      cmd->transitionImageLayout(
        mImages[mCurrentImageIndex],
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::PipelineStageFlagBits::eTransfer);
    }

    cmd->end();

    cmd->submit(
      {*renderFinishedSemaphore, *mImageAvailableSemaphores[mCurrentPresentIndex]},
      {2, vk::PipelineStageFlagBits::eColorAttachmentOutput},
      {*mCopyFinishedSemaphores[mCurrentPresentIndex]},
      *signalFence);
  }

  // present on mOutputWindow ----------------------------------------------------------------------
  {
    vk::SwapchainKHR swapChains[]     = {*mSwapchain};
    vk::Semaphore    waitSemaphores[] = {*mCopyFinishedSemaphores[mCurrentPresentIndex]};

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = waitSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapChains;
    presentInfo.pImageIndices      = &mCurrentImageIndex;

    try {
      result = mDevice->getQueue(QueueType::eGeneric).presentKHR(presentInfo);

      if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        // when does this happen?
        ILLUSION_ERROR << "out of date 1!" << std::endl;
      } else if (result != vk::Result::eSuccess) {
        // when does this happen?
        ILLUSION_ERROR << "out of date 2!" << std::endl;
      }
    } catch (vk::OutOfDateKHRError const& e) { mDirty = true; }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::chooseExtent() {
  auto capabilities = mDevice->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*mSurface);

  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    mExtent.x = capabilities.currentExtent.width;
    mExtent.y = capabilities.currentExtent.height;
  } else {

    // when does this happen?
    ILLUSION_WARNING << "TODO" << std::endl;
    mExtent = {500, 500};

    mExtent.x = std::max(
      capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, mExtent.x));
    mExtent.y = std::max(
      capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, mExtent.y));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::chooseFormat() {
  auto formats = mDevice->getPhysicalDevice()->getSurfaceFormatsKHR(*mSurface);

  if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
    mFormat = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    return;
  }

  for (const auto& format : formats) {
    if (
      format.format == vk::Format::eB8G8R8A8Unorm &&
      format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      mFormat = format;
      return;
    }
  }

  mFormat = formats[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::createSwapchain() {

  auto capabilities = mDevice->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*mSurface);
  auto formats      = mDevice->getPhysicalDevice()->getSurfaceFormatsKHR(*mSurface);
  auto presentModes = mDevice->getPhysicalDevice()->getSurfacePresentModesKHR(*mSurface);

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

  // choose minimum image count
  uint32_t imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR info;
  info.surface            = *mSurface;
  info.minImageCount      = imageCount;
  info.imageFormat        = mFormat.format;
  info.imageColorSpace    = mFormat.colorSpace;
  info.imageExtent.width  = mExtent.x;
  info.imageExtent.height = mExtent.y;
  info.imageArrayLayers   = 1;
  info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
  info.preTransform     = capabilities.currentTransform;
  info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  info.presentMode      = presentMode;
  info.clipped          = true;
  info.oldSwapchain     = nullptr; // this could be optimized
  info.imageSharingMode = vk::SharingMode::eExclusive;

  // this check should not be neccessary, but the validation layers complain
  // when only glfwGetPhysicalDevicePresentationSupport was used to check for
  // presentation support
  if (!mDevice->getPhysicalDevice()->getSurfaceSupportKHR(
        mDevice->getPhysicalDevice()->getQueueFamily(QueueType::eGeneric), *mSurface)) {
    ILLUSION_ERROR << "The selected queue family does not "
                   << "support presentation!" << std::endl;
  }

  mSwapchain = mDevice->createSwapChainKhr(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::createSemaphores() {
  for (auto const& i : mImages) {
    mImageAvailableSemaphores.push_back(mDevice->createSemaphore());
    mCopyFinishedSemaphores.push_back(mDevice->createSemaphore());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::createCommandBuffers() {
  for (auto const& i : mImages) {
    mPresentCommandBuffers.push_back(std::make_shared<CommandBuffer>(mDevice));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
