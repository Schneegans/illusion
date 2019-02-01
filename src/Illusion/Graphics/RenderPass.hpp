////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDER_PASS_HPP
#define ILLUSION_GRAPHICS_RENDER_PASS_HPP

#include "Device.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class RenderPass : public Core::StaticCreate<RenderPass>, public Core::NamedObject {

 public:
  struct Subpass {
    std::vector<uint32_t> mPreSubpasses;
    std::vector<uint32_t> mInputAttachments;
    std::vector<uint32_t> mOutputAttachments;
  };

  struct Attachment {
    BackedImagePtr        mImage;
    vk::ImageLayout       mInitialLayout;
    vk::ImageLayout       mFinalLayout;
    vk::AttachmentLoadOp  mLoadOp;
    vk::AttachmentStoreOp mStoreOp;
  };

  RenderPass(std::string const& name, DevicePtr const& device);
  virtual ~RenderPass();

  virtual void init();

  void addColorAttachment(Attachment const& attachment);
  void addDepthAttachment(Attachment const& attachment);

  void clearAttachments();

  bool                           hasDepthAttachment() const;
  std::vector<Attachment> const& getAttachments() const;

  void                        setSubpasses(std::vector<Subpass> const& subpasses);
  std::vector<Subpass> const& getSubpasses();

  glm::uvec2                getExtent() const;
  vk::FramebufferPtr const& getFramebuffer() const;
  vk::RenderPassPtr const&  getHandle() const;

 protected:
  DevicePtr mDevice;
  bool      mDirty = true;

 private:
  void createRenderPass();
  void createFramebuffer();

  vk::RenderPassPtr       mRenderPass;
  vk::FramebufferPtr      mFramebuffer;
  std::vector<Attachment> mAttachments;
  std::vector<Subpass>    mSubpasses;

  int32_t mDepthAttachment = -1;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_PASS_HPP
