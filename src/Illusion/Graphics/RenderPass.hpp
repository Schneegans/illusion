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
#include <optional>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This class wraps a vk::RenderPass and an according vk::Framebuffer. It can be used to add or   //
// delete attachments and subpasses. It keeps track of any modification via a dirty flag and will //
// re-create the render pass and the framebuffer as needed.                                       //
// The LazyRenderPass is a derived class which makes the usage a bit easier in many cases as it   //
// creates the framebuffer attachments on-the-fly.                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class RenderPass : public Core::StaticCreate<RenderPass>, public Core::NamedObject {

 public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // This struct is used to describe a subpass. All uint32_t's refer to the index of the          //
  // corresponding attachment.                                                                    //
  //////////////////////////////////////////////////////////////////////////////////////////////////

  struct Subpass {
    std::vector<uint32_t>   mPreSubpasses;
    std::vector<uint32_t>   mInputAttachments;
    std::vector<uint32_t>   mColorAttachments;
    std::optional<uint32_t> mDepthStencilAttachment;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // This struct describes an attachment. You have to provide a BackedImage, the image layout     //
  // this image will be in once the render pass is about to start, the image layout it will be    //
  // transitioned to during the render pass and whether the data has to loaded and / or stored.   //
  //////////////////////////////////////////////////////////////////////////////////////////////////

  struct Attachment {
    BackedImagePtr        mImage;
    vk::ImageLayout       mInitialLayout;
    vk::ImageLayout       mFinalLayout;
    vk::AttachmentLoadOp  mLoadOp;
    vk::AttachmentStoreOp mStoreOp;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////

  RenderPass(std::string const& name, DevicePtr device);
  virtual ~RenderPass();

  // This will initialize the contained vk::Framebuffer and vk::RenderPass. You do not really have
  // to call this as it will be called by the CommandBuffer when the render pass is begun. If you
  // call it on your own, you should be sure that the contained framebuffer and render pass are not
  // currently in use.
  virtual void init();

  // Returns the common extent of all attachments. This will return glm::uvec2(0) if no attachments
  // have been added.
  glm::uvec2 getExtent() const;

  // Returns the wrapped vk::Framebuffer. If this returns an empty pointer, you will have to call
  // init() before.
  vk::FramebufferPtr const& getFramebuffer() const;

  // Returns the wrapped vk::RenderPass. If this returns an empty pointer, you will have to call
  // init() before.
  vk::RenderPassPtr const& getHandle() const;

  // attachment api --------------------------------------------------------------------------------

  // Adding an attachment with a size which differs from previously added attachments will throw a
  // std::runtime_error.
  void                           addAttachment(Attachment const& attachment);
  void                           setAttachments(std::vector<Attachment> const& attachments);
  std::vector<Attachment> const& getAttachments() const;
  void                           clearAttachments();

  // subpass api -----------------------------------------------------------------------------------

  // When no subpasses are defined, a default subpass will be created. This will use all attachments
  // with a color vk::Format as color attachments and the last non-color attachment as depth
  // attachment.
  void                        addSubpass(Subpass const& subpass);
  void                        setSubpasses(std::vector<Subpass> const& subpasses);
  std::vector<Subpass> const& getSubpasses() const;
  void                        clearSubpasses();

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
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_PASS_HPP
