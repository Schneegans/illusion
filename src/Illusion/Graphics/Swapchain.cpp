////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Swapchain.hpp"

#include "../Core/Logger.hpp"
#include "BackedImage.hpp"
#include "CommandBuffer.hpp"
#include "PhysicalDevice.hpp"
#include "Window.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

Swapchain::Swapchain(
    std::string const& name, DevicePtr const& device, vk::SurfaceKHRPtr const& surface)
    : Core::NamedObject(name)
    , mDevice(device)
    , mSurface(surface) {

  Core::Logger::traceCreation("Swapchain", getName());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Swapchain::~Swapchain() {
  Core::Logger::traceDeletion("Swapchain", getName());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::setEnableVsync(bool enable) {
  if (enable != mEnableVsync) {
    mEnableVsync = enable;
    markDirty();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::markDirty() {
  mDirty = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::uvec2 const& Swapchain::getExtent() const {
  return mExtent;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::present(BackedImagePtr const& image,
    vk::SemaphorePtr const& renderFinishedSemaphore, vk::FencePtr const& signalFence) {

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

    auto cmd = std::make_shared<CommandBuffer>("Transition swapchain image layouts", mDevice);
    cmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    for (auto const& image : mImages) {
      cmd->transitionImageLayout(image, vk::ImageLayout::eUndefined,
          vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits::eTopOfPipe,
          vk::PipelineStageFlagBits::eTransfer);
    }
    cmd->end();
    cmd->submit();
    cmd->waitIdle();

    createSemaphores();
    createCommandBuffers();

    mDirty = false;
  }

  mCurrentPresentIndex = (mCurrentPresentIndex + 1) % mImages.size();

  auto result =
      mDevice->getHandle()->acquireNextImageKHR(*mSwapchain, std::numeric_limits<uint64_t>::max(),
          *mImageAvailableSemaphores[mCurrentPresentIndex], nullptr, &mCurrentImageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    // mark dirty and call this method again
    mDirty = true;
    present(image, renderFinishedSemaphore, signalFence);
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    Core::Logger::error() << "Suboptimal swap chain!" << std::endl;
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
    if (image->mImageInfo.samples != vk::SampleCountFlagBits::e1) {
      vk::ImageResolve region;
      region.srcSubresource = subResource;
      region.dstSubresource = subResource;
      region.srcOffset      = vk::Offset3D(0, 0, 0);
      region.dstOffset      = vk::Offset3D(0, 0, 0);
      region.extent.width   = image->mImageInfo.extent.width;
      region.extent.height  = image->mImageInfo.extent.height;
      region.extent.depth   = 1;

      cmd->resolveImage(*image->mImage, vk::ImageLayout::eTransferSrcOptimal,
          mImages[mCurrentImageIndex], vk::ImageLayout::eTransferDstOptimal, region);
    }
    // else do an image blit
    else {
      cmd->transitionImageLayout(*image->mImage, vk::ImageLayout::eColorAttachmentOptimal,
          vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput,
          vk::PipelineStageFlagBits::eTransfer);
      cmd->transitionImageLayout(mImages[mCurrentImageIndex], vk::ImageLayout::ePresentSrcKHR,
          vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer,
          vk::PipelineStageFlagBits::eTransfer);
      vk::ImageBlit info;
      cmd->blitImage(*image->mImage, mImages[mCurrentImageIndex],
          {image->mImageInfo.extent.width, image->mImageInfo.extent.height}, mExtent,
          vk::Filter::eNearest);
      cmd->transitionImageLayout(*image->mImage, vk::ImageLayout::eTransferSrcOptimal,
          vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits::eTransfer,
          vk::PipelineStageFlagBits::eColorAttachmentOutput);
      cmd->transitionImageLayout(mImages[mCurrentImageIndex], vk::ImageLayout::eTransferDstOptimal,
          vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits::eTransfer,
          vk::PipelineStageFlagBits::eTransfer);
    }

    cmd->end();

    cmd->submit({renderFinishedSemaphore, mImageAvailableSemaphores[mCurrentPresentIndex]},
        {2, vk::PipelineStageFlagBits::eColorAttachmentOutput},
        {mCopyFinishedSemaphores[mCurrentPresentIndex]}, signalFence);
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
        Core::Logger::error() << "out of date 1!" << std::endl;
      } else if (result != vk::Result::eSuccess) {
        // when does this happen?
        Core::Logger::error() << "out of date 2!" << std::endl;
      }
    } catch (...) { mDirty = true; }
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
    Core::Logger::warning() << "TODO" << std::endl;
    mExtent = {500, 500};

    mExtent.x = std::max(
        capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, mExtent.x));
    mExtent.y = std::max(capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, mExtent.y));
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
    if (format.format == vk::Format::eB8G8R8A8Unorm &&
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
  vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

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
    Core::Logger::error() << "The selected queue family does not "
                          << "support presentation!" << std::endl;
  }

  mSwapchain = mDevice->createSwapChainKhr(getName(), info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::createSemaphores() {
  for (size_t i(0); i < mImages.size(); ++i) {
    mImageAvailableSemaphores.push_back(
        mDevice->createSemaphore("ImageAvailable " + std::to_string(i) + " of " + getName()));
    mCopyFinishedSemaphores.push_back(
        mDevice->createSemaphore("ImageCopyFinished " + std::to_string(i) + " of " + getName()));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Swapchain::createCommandBuffers() {
  for (size_t i(0); i < mImages.size(); ++i) {
    mPresentCommandBuffers.push_back(std::make_shared<CommandBuffer>(
        "Presentation " + std::to_string(i) + " of " + getName(), mDevice));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
