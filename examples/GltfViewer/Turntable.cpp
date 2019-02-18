////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE

#include "Turntable.hpp"

#include <Illusion/Graphics/Window.hpp>

#include <glm/gtx/transform.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////

Turntable::Turntable(Illusion::Graphics::WindowPtr const& window) {

  window->sOnMouseEvent.connect([this, &window](Illusion::Input::MouseEvent const& e) {
    if (e.mType == Illusion::Input::MouseEvent::Type::eMove) {
      static int32_t lastX = e.mX;
      static int32_t lastY = e.mY;

      int32_t dX = lastX - e.mX;
      int32_t dY = lastY - e.mY;

      if (window->buttonPressed(Illusion::Input::Button::eButton1)) {
        mCameraPolar.x += dX * 0.005f;
        mCameraPolar.y += dY * 0.005f;

        mCameraPolar.y = glm::clamp(
            mCameraPolar.y, -glm::pi<float>() * 0.5f + 0.1f, glm::pi<float>() * 0.5f - 0.1f);

      } else if (window->buttonPressed(Illusion::Input::Button::eButton2)) {
        mCameraOffset.x -= dX * 0.002f;
        mCameraOffset.y += dY * 0.002f;
      }

      lastX = e.mX;
      lastY = e.mY;
    } else if (e.mType == Illusion::Input::MouseEvent::Type::eScroll) {
      mCameraPolar.z -= e.mY * 0.01;
      mCameraPolar.z = std::max(mCameraPolar.z, 0.01f);
    }

    return true;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::vec4 Turntable::getCameraPosition() const {
  auto pos = glm::inverse(getViewMatrix()) * glm::vec4(0, 0, 0, 1);
  return pos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::mat4 Turntable::getViewMatrix() const {
  auto position =
      glm::vec3(std::cos(mCameraPolar.y) * std::sin(mCameraPolar.x), -std::sin(mCameraPolar.y),
          std::cos(mCameraPolar.y) * std::cos(mCameraPolar.x)) *
      mCameraPolar.z;
  auto viewMatrix = glm::lookAt(position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
  return glm::translate(glm::vec3(mCameraOffset, 0)) * viewMatrix;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
