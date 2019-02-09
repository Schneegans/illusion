////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_EXAMPLES_DEFERRED_RENDERING_LIGHTS_HPP
#define ILLUSION_EXAMPLES_DEFERRED_RENDERING_LIGHTS_HPP

#include <Illusion/Graphics/FrameResource.hpp>
#include <Illusion/Graphics/fwd.hpp>

#include <glm/glm.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Lights {
 public:
  Lights(Illusion::Graphics::DevicePtr const&          device,
      Illusion::Graphics::FrameResourceIndexPtr const& frameIndex, uint32_t lightCount);

  void update(float time, glm::mat4 const& matMVP);
  void draw(Illusion::Graphics::CommandBufferPtr const& cmd);

 private:
  struct Light {
    glm::vec4 mPosition;
    glm::vec4 mColor;
  };

  std::vector<Light>                  mLights;
  glm::mat4                           mMatMVP;
  Illusion::Graphics::BackedBufferPtr mPositionBuffer;
  Illusion::Graphics::BackedBufferPtr mIndexBuffer;
  Illusion::Graphics::ShaderPtr       mShader;

  Illusion::Graphics::FrameResource<Illusion::Graphics::CoherentBufferPtr> mLightBuffer;
};

#endif // ILLUSION_EXAMPLES_DEFERRED_RENDERING_LIGHTS_HPP
