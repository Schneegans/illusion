////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "Window.hpp"

#include "../Core/Logger.hpp"
#include "DisplayPass.hpp"
#include "Engine.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

Window::Window(std::shared_ptr<Engine> const& engine, std::shared_ptr<Context> const& context)
  : mEngine(engine)
  , mContext(context) {

  ILLUSION_TRACE << "Creating Window." << std::endl;

  pCursor.onChange().connect([this](Cursor cursor) {
    if (mCursor) { glfwDestroyCursor(mCursor); }

    switch (cursor) {
    case Cursor::CROSS:
      mCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
      break;
    case Cursor::HAND:
      mCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
      break;
    case Cursor::IBEAM:
      mCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
      break;
    case Cursor::VRESIZE:
      mCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
      break;
    case Cursor::HRESIZE:
      mCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
      break;
    default:
      mCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
      break;
    }

    if (mWindow) { glfwSetCursor(mWindow, mCursor); }

    return true;
  });

  pLockAspect.onChange().connect([this](bool val) {
    if (mWindow) {
      if (val) {
        glfwSetWindowAspectRatio(mWindow, pSize().x, pSize().y);
      } else {
        glfwSetWindowAspectRatio(mWindow, GLFW_DONT_CARE, GLFW_DONT_CARE);
      }
    }
    return true;
  });

  pFullscreen.onChange().connect([this](bool fullscreen) {
    if (mWindow) {
      if (fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwGetWindowPos(mWindow, &mOrigPos.x, &mOrigPos.y);
        glfwGetWindowSize(mWindow, &mOrigSize.x, &mOrigSize.y);
        glfwSetWindowMonitor(
          mWindow, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);

      } else {
        glfwSetWindowMonitor(mWindow, nullptr, mOrigPos.x, mOrigPos.y, mOrigSize.x, mOrigSize.y, 0);
      }
    }
    return true;
  });

  pVsync.onChange().connect([this](bool vsync) {
    if (mDisplayPass) { mDisplayPass->setEnableVsync(vsync); }
    return true;
  });

  pTitle.onChange().connect([this](std::string const& title) {
    if (mWindow) { glfwSetWindowTitle(mWindow, title.c_str()); }
    return true;
  });

  pHideCursor.onChange().connect([this](bool hide) {
    if (mWindow) {
      glfwSetInputMode(mWindow, GLFW_CURSOR, hide ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    }
    return true;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Window::~Window() {
  ILLUSION_TRACE << "Deleting Window." << std::endl;

  if (mCursor) { glfwDestroyCursor(mCursor); }
  close();

  mContext->getDevice()->waitIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::open() {
  if (!mWindow) {

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (pFullscreen()) {
      auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
      mWindow   = glfwCreateWindow(
        mode->width, mode->height, pTitle().c_str(), glfwGetPrimaryMonitor(), nullptr);
    } else {
      mWindow = glfwCreateWindow(pSize().x, pSize().y, pTitle().c_str(), nullptr, nullptr);
    }

    mSurface     = mEngine->createSurface(mWindow);
    mDisplayPass = std::make_shared<DisplayPass>(mContext, mSurface);

    pLockAspect.touch();
    pHideCursor.touch();
    pVsync.touch();
    pCursor.touch();

    glfwSetWindowUserPointer(mWindow, this);

    glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* w) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnClose.emit();
    });

    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* w, int width, int height) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      // issuing this here reduces flickering during resize but may result in more swapchain
      // recreations than abolutely neccessary
      window->pSize = glm::uvec2(width, height);
      window->mDisplayPass->markSwapChainDirty();
    });

    glfwSetKeyCallback(mWindow, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnKeyEvent.emit(Input::KeyEvent(key, scancode, action, mods));
    });

    glfwSetCursorPosCallback(mWindow, [](GLFWwindow* w, double x, double y) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnMouseEvent.emit(Input::MouseEvent(static_cast<int>(x), static_cast<int>(y)));
    });

    glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* w, int button, int action, int mods) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnMouseEvent.emit(Input::MouseEvent(button, action == GLFW_PRESS));
    });

    glfwSetScrollCallback(mWindow, [](GLFWwindow* w, double x, double y) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnMouseEvent.emit(Input::MouseEvent(static_cast<int>(y * 10.0)));
    });

    glfwSetCharModsCallback(mWindow, [](GLFWwindow* w, unsigned c, int mods) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnKeyEvent.emit(Input::KeyEvent(c, mods));
    });

  } else {
    ILLUSION_WARNING << "Attempting to open an already opened window!" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::close() {
  if (mWindow) {
    glfwDestroyWindow(mWindow);
    mWindow = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::shouldClose() const { return glfwWindowShouldClose(mWindow); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::processInput() {
  if (mWindow) {
    glfwPollEvents();
    updateJoysticks();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::keyPressed(Input::Key key) const {
  if (!mWindow) { return false; }

  return glfwGetKey(mWindow, (int)key) == GLFW_PRESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::buttonPressed(Input::Button button) const {
  if (!mWindow) { return false; }

  return glfwGetMouseButton(mWindow, (int)button) == GLFW_PRESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Window::joyAxis(int joyStick, int axis) {
  if (
    joyStick >= static_cast<int>(Input::JoystickId::JOYSTICK_NUM) ||
    axis >= static_cast<int>(Input::JoystickAxisId::JOYSTICK_AXIS_NUM)) {
    return 0;
  }
  if (!glfwJoystickPresent(joyStick)) { return 0; }
  return mJoystickAxisCache[joyStick][axis];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::vec2 Window::getCursorPos() const {
  if (!mWindow) { return glm::vec2(0.f, 0.f); }
  double x, y;
  glfwGetCursorPos(mWindow, &x, &y);
  return glm::vec2(x, y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::updateJoysticks() {
  float       changedThreshold(0.01f);
  const float minThreshold(0.15f);
  const float maxThreshold(0.9f);

  const int joystickNum(static_cast<int>(Input::JoystickId::JOYSTICK_NUM));
  for (int joy(0); joy < joystickNum; ++joy) {
    if (glfwJoystickPresent(joy)) {
      Input::JoystickId joyId(static_cast<Input::JoystickId>(joy));
      int               axesCount(0);
      auto              axesArray(glfwGetJoystickAxes(joy, &axesCount));

      for (int axis(0); axis < axesCount; ++axis) {
        float axisValue(axesArray[axis]);

// XBOX controller left and right trigger are both mapped to axis 2 on windows.
#if defined(WIN32)
        if (axis == 2) {
          if (axisValue < 0) {
            axis      = 5;
            axisValue = -axisValue;
          }
        }
#else
        if (axis == 2 || axis == 5) { axisValue = (axisValue + 1) * 0.5; }
#endif

        int sign(axisValue < 0.f ? -1 : 1);
        axisValue = std::abs(axisValue);

        axisValue = (axisValue - minThreshold) / (maxThreshold - minThreshold);
        axisValue = sign * std::min(1.f, std::max(0.f, axisValue));

        if (axisValue == 0.f || axisValue == 1.f || axisValue == -1.f) { changedThreshold = 0.f; }

        if (std::abs(axisValue - mJoystickAxisCache[joy][axis]) > changedThreshold) {
          Input::JoystickAxisId axisId(static_cast<Input::JoystickAxisId>(axis));
          sOnJoystickAxisChanged.emit(joyId, axisId, axisValue);
          mJoystickAxisCache[joy][axis] = axisValue;
        }
      }

      int  buttonCount(0);
      auto buttonArray(glfwGetJoystickButtons(joy, &buttonCount));
      for (int button(0); button < buttonCount; ++button) {
        Input::JoystickButtonId buttonId(static_cast<Input::JoystickButtonId>(button));
        int                     buttonValue(static_cast<int>(buttonArray[button]));

        if (buttonValue != mJoystickButtonCache[joy][button]) {

          if (buttonValue == 0) {
            sOnJoystickButtonReleased.emit(joyId, buttonId);
          } else if (buttonValue == 1) {
            sOnJoystickButtonPressed.emit(joyId, buttonId);
          }

          mJoystickButtonCache[joy][button] = buttonValue;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
