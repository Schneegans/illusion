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

#include <glm/glm.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Framebuffer {
 public:
  // -------------------------------------------------------------------------------- public methods
  Framebuffer(
    std::shared_ptr<Device> const&         device,
    std::shared_ptr<vk::RenderPass> const& renderPass,
    glm::uvec2 const&                      extent,
    std::vector<vk::Format> const&         attachments);

  virtual ~Framebuffer();

  std::shared_ptr<vk::Framebuffer> const&          getFramebuffer() const { return mFramebuffer; }
  std::vector<std::shared_ptr<BackedImage>> const& getImages() const { return mImageStore; }

 private:
  std::shared_ptr<Device>         mDevice;
  std::shared_ptr<vk::RenderPass> mRenderPass;
  glm::uvec2                      mExtent;

  std::shared_ptr<vk::Framebuffer>            mFramebuffer;
  std::vector<std::shared_ptr<vk::ImageView>> mImageViewStore;
  std::vector<std::shared_ptr<BackedImage>>   mImageStore;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDERTARGET_HPP
