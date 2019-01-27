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
  struct SubPass {
    std::vector<uint32_t> mPreSubPasses;
    std::vector<uint32_t> mInputAttachments;
    std::vector<uint32_t> mOutputAttachments;
  };

  RenderPass(std::string const& name, DevicePtr const& device);
  virtual ~RenderPass();

  void init();

  void                           addAttachment(vk::Format format);
  bool                           hasDepthAttachment() const;
  std::vector<vk::Format> const& getFrameBufferAttachmentFormats() const;

  void setSubPasses(std::vector<SubPass> const& subPasses);

  void              setExtent(glm::uvec2 const& extent);
  glm::uvec2 const& getExtent() const;

  std::vector<BackedImagePtr> const& getAttachments() const;

  vk::FramebufferPtr const& getFramebuffer() const;
  vk::RenderPassPtr const&  getHandle() const;

 private:
  void createRenderPass();
  void createFramebuffer();

  DevicePtr                   mDevice;
  vk::RenderPassPtr           mRenderPass;
  vk::FramebufferPtr          mFramebuffer;
  std::vector<vk::Format>     mFrameBufferAttachmentFormats;
  std::vector<BackedImagePtr> mImageStore;
  std::vector<SubPass>        mSubPasses;
  bool                        mAttachmentsDirty = true;
  glm::uvec2                  mExtent           = {100, 100};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_PASS_HPP
