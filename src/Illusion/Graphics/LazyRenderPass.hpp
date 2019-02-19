////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_LAZY_RENDER_PASS_HPP
#define ILLUSION_GRAPHICS_LAZY_RENDER_PASS_HPP

#include "RenderPass.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The LazyRenderPass is a special version of the RenderPass which handles creation of            //
// BackedImages for the attachments for you. You specify an extent and a set of vk::Formats and   //
// the corresponding BackedImages will be created once init() gets called.                        //
////////////////////////////////////////////////////////////////////////////////////////////////////

class LazyRenderPass : public RenderPass, public Core::StaticCreate<LazyRenderPass> {

 public:
  // Make sure that we use the correct static create method.
  using Core::StaticCreate<LazyRenderPass>::create;

  LazyRenderPass(std::string const& name, DevicePtr const& device);
  virtual ~LazyRenderPass() override;

  // This will initialize the contained vk::Framebuffer and vk::RenderPass. You do not really have
  // to call this as it will be called by the CommandBuffer when the render pass is begun. If you
  // call it on your own, you should be sure that the contained framebuffer and render pass are not
  // currently in use.
  void init() override;

  // When no subpasses are defined, a default subpass will be created. This will use all attachments
  // with a color vk::Format as color attachments and the last non-color attachment as depth
  // attachment.
  void addAttachment(vk::Format format);

  // Default extent is 100 by 100. Setting this at runtime will trigger a re-creation of all
  // attachments.
  void setExtent(glm::uvec2 const& extent);

 private:
  void createImages();

  std::vector<vk::Format> mAttachmentFormats;
  glm::uvec2              mExtent = {100, 100};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_LAZY_RENDER_PASS_HPP
