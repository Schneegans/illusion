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
////////////////////////////////////////////////////////////////////////////////////////////////////

class LazyRenderPass : public RenderPass, public Core::StaticCreate<LazyRenderPass> {

 public:
  using Core::StaticCreate<LazyRenderPass>::create;

  LazyRenderPass(std::string const& name, DevicePtr const& device);
  virtual ~LazyRenderPass();

  void init() override;
  void addAttachment(vk::Format format);
  void setExtent(glm::uvec2 const& extent);

 private:
  void createImages();

  std::vector<vk::Format> mAttachmentFormats;
  glm::uvec2              mExtent = {100, 100};
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_LAZY_RENDER_PASS_HPP
