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

#include "Device.hpp"
#include "Framebuffer.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class RenderPass {

 public:
  struct SubPass {
    std::vector<uint32_t> mPreSubPasses;
    std::vector<uint32_t> mInputAttachments;
    std::vector<uint32_t> mOutputAttachments;
  };

  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static RenderPassPtr create(Args&&... args) {
    return std::make_shared<RenderPass>(args...);
  };

  RenderPass(DevicePtr const& device);
  virtual ~RenderPass();

  void init();

  void                           addAttachment(vk::Format format);
  bool                           hasDepthAttachment() const;
  std::vector<vk::Format> const& getFrameBufferAttachmentFormats() const;

  void setSubPasses(std::vector<SubPass> const& subPasses);

  void              setExtent(glm::uvec2 const& extent);
  glm::uvec2 const& getExtent() const;

  FramebufferPtr const&    getFramebuffer() const;
  vk::RenderPassPtr const& getHandle() const;

 private:
  vk::RenderPassPtr createRenderPass() const;

  DevicePtr               mDevice;
  vk::RenderPassPtr       mRenderPass;
  FramebufferPtr          mFramebuffer;
  std::vector<vk::Format> mFrameBufferAttachmentFormats;
  std::vector<SubPass>    mSubPasses;
  bool                    mAttachmentsDirty = true;
  glm::uvec2              mExtent           = {100, 100};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_PASS_HPP
