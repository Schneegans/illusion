////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDER_PASS_HPP
#define ILLUSION_GRAPHICS_RENDER_PASS_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Context.hpp"
#include "RenderTarget.hpp"

#include <functional>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class RenderPass {

 public:
  struct SubPass {
    std::vector<uint32_t> mPreSubPasses;
    std::vector<uint32_t> mInputAttachments;
    std::vector<uint32_t> mOutputAttachments;
  };

  // -------------------------------------------------------------------------------- public methods
  RenderPass(std::shared_ptr<Context> const& context);
  virtual ~RenderPass();

  virtual void render();

  virtual void addAttachment(vk::Format format);

  void setSubPasses(std::vector<SubPass> const& subPasses);

  std::shared_ptr<vk::Pipeline> createPipeline(vk::GraphicsPipelineCreateInfo info) const;

  void                setExtent(vk::Extent2D const& extent);
  vk::Extent2D const& getExtent() const;

  void     setRingBufferSize(uint32_t extent);
  uint32_t getRingBufferSize() const;

  void executeAfter(std::shared_ptr<RenderPass> const& other);
  void executeBefore(std::shared_ptr<RenderPass> const& other);

  std::function<void(vk::CommandBuffer&)>           beforeFunc;
  std::function<void(vk::CommandBuffer&, uint32_t)> drawFunc;

  bool hasDepthAttachment() const;

 protected:
  // ----------------------------------------------------------------------------- protected methods
  void setSwapchainInfo(std::vector<vk::Image> const& images, vk::Format format);

  // ----------------------------------------------------------------------------- protected members
  std::shared_ptr<Context> mContext;

  std::vector<std::weak_ptr<vk::Semaphore>> mWaitSemaphores;
  std::shared_ptr<vk::Semaphore>            mSignalSemaphore;
  std::shared_ptr<vk::RenderPass>           mRenderPass;

  // ring buffer of for command buffer and associated framebuffers
  uint32_t mCurrentRingBufferIndex = 0;

 protected:
  // ------------------------------------------------------------------------------- private methods
  std::shared_ptr<vk::Semaphore>             createSignalSemaphore() const;
  std::vector<std::shared_ptr<vk::Fence>>    createFences() const;
  std::vector<vk::CommandBuffer>             createCommandBuffers() const;
  std::shared_ptr<vk::RenderPass>            createRenderPass() const;
  std::vector<std::shared_ptr<BackedImage>>  createFramebufferAttachments() const;
  std::vector<std::shared_ptr<RenderTarget>> createRenderTargets() const;

  // ------------------------------------------------------------------------------- private members
  std::vector<std::shared_ptr<vk::Fence>>    mFences;
  std::vector<vk::CommandBuffer>             mCommandBuffers;
  std::vector<std::shared_ptr<RenderTarget>> mRenderTargets;

  // when this RenderPass is used for presentation on a Swapchain, these members will define the
  // first framebuffer attachment for each ringbuffer element
  std::vector<vk::Image> mSwapchainImages;
  vk::Format             mSwapchainFormat;

  // one for each framebuffer attachment, will be appended after the swapchain image
  std::vector<vk::Format>                   mFrameBufferAttachmentFormats;
  std::vector<std::shared_ptr<BackedImage>> mFrameBufferAttachments;

  std::vector<SubPass> mSubPasses;

  bool mAttachmentsDirty    = true;
  bool mRingbufferSizeDirty = true;

  vk::Extent2D mExtent;
  uint32_t     mRingbufferSize = 0;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_PASS_HPP
