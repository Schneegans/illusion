////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDERTARGET_HPP
#define ILLUSION_GRAPHICS_RENDERTARGET_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class RenderTarget {
 public:
  struct AttachmentDescription {
    vk::Format                 mFormat;
    std::shared_ptr<vk::Image> mImage;
  };

  // -------------------------------------------------------------------------------- public methods
  RenderTarget(
    std::shared_ptr<Engine> const&            engine,
    std::shared_ptr<vk::RenderPass> const&    renderPass,
    vk::Extent2D const&                       extent,
    std::vector<AttachmentDescription> const& attachmentDescriptions);

  virtual ~RenderTarget();

  std::shared_ptr<vk::Framebuffer> const& getFramebuffer() const { return mFramebuffer; }

 private:
  std::shared_ptr<Engine>         mEngine;
  std::shared_ptr<vk::RenderPass> mRenderPass;
  vk::Extent2D                    mExtent;

  std::shared_ptr<vk::Framebuffer> mFramebuffer;

  std::vector<std::shared_ptr<vk::ImageView>> mImageViewStore;
  std::vector<std::shared_ptr<vk::Image>>     mImageStore;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDERTARGET_HPP
