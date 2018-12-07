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

  RenderPass(std::shared_ptr<Device> const& device);
  virtual ~RenderPass();

  void init();

  void                                   addAttachment(vk::Format format);
  bool                                   hasDepthAttachment() const;
  void                                   setSubPasses(std::vector<SubPass> const& subPasses);
  void                                   setExtent(glm::uvec2 const& extent);
  glm::uvec2 const&                      getExtent() const;
  std::shared_ptr<Framebuffer> const&    getFramebuffer() const;
  std::shared_ptr<vk::RenderPass> const& getHandle() const;

 private:
  std::shared_ptr<vk::RenderPass> createRenderPass() const;

  std::shared_ptr<Device>         mDevice;
  std::shared_ptr<vk::RenderPass> mRenderPass;
  std::shared_ptr<Framebuffer>    mFramebuffer;
  std::vector<vk::Format>         mFrameBufferAttachmentFormats;
  std::vector<SubPass>            mSubPasses;
  bool                            mAttachmentsDirty = true;
  glm::uvec2                      mExtent           = {100, 100};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_PASS_HPP
