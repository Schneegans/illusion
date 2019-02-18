////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_EXAMPLES_GLTF_VIEWER_TURNTABLE_HPP
#define ILLUSION_EXAMPLES_GLTF_VIEWER_TURNTABLE_HPP

#include <Illusion/Graphics/fwd.hpp>

#include <glm/glm.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Turntable {
 public:
  Turntable(Illusion::Graphics::WindowPtr const& window);

  glm::vec4 getCameraPosition() const;
  glm::mat4 getViewMatrix() const;

 private:
  glm::vec3 mCameraPolar  = glm::vec3(0.f, 0.f, 1.5f);
  glm::vec2 mCameraOffset = glm::vec2(0.f, 0.f);
};

#endif // ILLUSION_EXAMPLES_GLTF_VIEWER_TURNTABLE_HPP
