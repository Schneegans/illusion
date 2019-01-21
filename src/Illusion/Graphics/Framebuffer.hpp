////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDERTARGET_HPP
#define ILLUSION_GRAPHICS_RENDERTARGET_HPP

#include "../Core/NamedObject.hpp"
#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

#include <glm/glm.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Framebuffer : public Core::StaticCreate<Framebuffer>, public Core::NamedObject {
 public:
  Framebuffer(std::string const& name, DevicePtr const& device, vk::RenderPassPtr const& renderPass,
      glm::uvec2 const& extent, std::vector<vk::Format> const& attachments);

  virtual ~Framebuffer();

  vk::FramebufferPtr const& getHandle() const {
    return mFramebuffer;
  }
  std::vector<BackedImagePtr> const& getImages() const {
    return mImageStore;
  }

 private:
  DevicePtr         mDevice;
  vk::RenderPassPtr mRenderPass;
  glm::uvec2        mExtent;

  vk::FramebufferPtr          mFramebuffer;
  std::vector<BackedImagePtr> mImageStore;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDERTARGET_HPP
