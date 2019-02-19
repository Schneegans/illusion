////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_EXAMPLES_DEFERRED_RENDERING_FLOOR_HPP
#define ILLUSION_EXAMPLES_DEFERRED_RENDERING_FLOOR_HPP

#include <Illusion/Graphics/FrameResource.hpp>
#include <Illusion/Graphics/fwd.hpp>

#include <glm/glm.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Floor {
 public:
  Floor(Illusion::Graphics::DevicePtr const& device);

  void update(glm::mat4 const& matVP);
  void draw(Illusion::Graphics::CommandBufferPtr const& cmd);

 private:
  glm::mat4                      mMatVP{};
  Illusion::Graphics::TexturePtr mAlbedoTexture;
  Illusion::Graphics::TexturePtr mNormalTexture;
  Illusion::Graphics::ShaderPtr  mShader;
};

#endif // ILLUSION_EXAMPLES_DEFERRED_RENDERING_FLOOR_HPP
