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
  struct SubpassInfo {
    std::vector<uint32_t> mPreSubpasses;
    std::vector<uint32_t> mInputAttachments;
    std::vector<uint32_t> mOutputAttachments;
  };

  RenderPass(std::string const& name, DevicePtr const& device);
  virtual ~RenderPass();

  virtual void init();

  void addAttachment(BackedImagePtr const& image);

  virtual bool hasDepthAttachment() const;

  void setSubpasses(std::vector<SubpassInfo> const& subpasses);

  virtual glm::uvec2 getExtent() const;

  std::vector<BackedImagePtr> const& getAttachments() const;
  vk::FramebufferPtr const&          getFramebuffer() const;
  vk::RenderPassPtr const&           getHandle() const;

 protected:
  DevicePtr                   mDevice;
  std::vector<BackedImagePtr> mImageStore;
  bool                        mAttachmentsDirty = true;

 private:
  void createRenderPass();
  void createFramebuffer();

  vk::RenderPassPtr        mRenderPass;
  vk::FramebufferPtr       mFramebuffer;
  std::vector<SubpassInfo> mSubpasses;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_PASS_HPP
