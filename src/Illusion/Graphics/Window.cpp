////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Window.hpp"

#include "../Core/Logger.hpp"
#include "Instance.hpp"
#include "Swapchain.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

Window::Window(std::string const& name, InstanceConstPtr instance, DeviceConstPtr device)
    : Core::NamedObject(name)
    , mInstance(std::move(instance))
    , mDevice(std::move(device)) {

  // Change the mouse pointer when pCursor is changed.
  pCursor.onChange().connect([this](Cursor cursor) {
    if (mCursor != nullptr) {
      glfwDestroyCursor(mCursor);
    }

    // clang-format off
    switch (cursor) {
      case Cursor::eCross:   mCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR); break;
      case Cursor::eHand:    mCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);      break;
      case Cursor::eIBeam:   mCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);     break;
      case Cursor::eVResize: mCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);   break;
      case Cursor::eHResize: mCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);   break;
      default:               mCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);     break;
    }
    // clang-format on

    if (mWindow != nullptr) {
      glfwSetCursor(mWindow, mCursor);
    }

    return true;
  });

  // Lock / unlock window's aspecti ratio when requested.
  pLockAspect.onChange().connect([this](bool val) {
    if (mWindow != nullptr) {
      if (val) {
        glfwSetWindowAspectRatio(mWindow, pExtent().x, pExtent().y);
      } else {
        glfwSetWindowAspectRatio(mWindow, GLFW_DONT_CARE, GLFW_DONT_CARE);
      }
    }
    return true;
  });

  // Toggle fullscreen when requested. When going fullscreen, the original window position and size
  // is stored in mOrigPos and mOrigSize so that the original window state can be restored when
  // leaving fullscreen mode.
  pFullscreen.onChange().connect([this](bool fullscreen) {
    if (mWindow != nullptr) {
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

  // Tell our swapchain thar v-sync has been changed.
  pVsync.onChange().connect([this](bool vsync) {
    if (mSwapchain) {
      mSwapchain->setEnableVsync(vsync);
    }
    return true;
  });

  // Set the window's title when the pTitle property changes.
  pTitle.onChange().connect([this](std::string const& title) {
    if (mWindow != nullptr) {
      glfwSetWindowTitle(mWindow, title.c_str());
    }
    return true;
  });

  // Optionally hide the mouse pointer when it is over the window.
  pHideCursor.onChange().connect([this](bool hide) {
    if (mWindow != nullptr) {
      glfwSetInputMode(mWindow, GLFW_CURSOR, hide ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    }
    return true;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Window::~Window() {
  if (mCursor != nullptr) {
    glfwDestroyCursor(mCursor);
  }

  // Descrutor closes the window.
  close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::open() {
  if (mWindow == nullptr) {

    // We will use Vulkan, no context is required.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (pFullscreen()) {
      auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
      mWindow   = glfwCreateWindow(
          mode->width, mode->height, pTitle().c_str(), glfwGetPrimaryMonitor(), nullptr);
    } else {
      mWindow = glfwCreateWindow(pExtent().x, pExtent().y, pTitle().c_str(), nullptr, nullptr);
    }

    // Create a surface and a swapchain for the window.
    mSurface   = mInstance->createSurface("Surface of " + getName(), mWindow);
    mSwapchain = std::make_shared<Swapchain>("Swapchain of " + getName(), mDevice, mSurface);

    // Initialize some aspects of the window by triggering the onChange() signal of our properties.
    pLockAspect.touch();
    pHideCursor.touch();
    pVsync.touch();
    pCursor.touch();

    // Store a pointer to "this" as user pointer for the glfw window. This is used to access
    // properties of "this" in the glfw callbacks below.
    glfwSetWindowUserPointer(mWindow, this);

    glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* w) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnClose.emit();
    });

    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* w, int width, int height) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->pExtent = glm::uvec2(width, height);
      // issuing this here reduces flickering during resize but may result in more swapchain
      // recreations than abolutely neccessary
      window->mSwapchain->markDirty();
    });

    glfwSetKeyCallback(mWindow, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnKeyEvent.emit(Input::KeyEvent(key, scancode, action, mods));
    });

    glfwSetCursorPosCallback(mWindow, [](GLFWwindow* w, double x, double y) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnMouseEvent.emit(Input::MouseEvent(static_cast<int>(x), static_cast<int>(y)));
    });

    glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* w, int button, int action, int /*mods*/) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnMouseEvent.emit(Input::MouseEvent(button, action == GLFW_PRESS));
    });

    glfwSetScrollCallback(mWindow, [](GLFWwindow* w, double /*x*/, double y) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnMouseEvent.emit(Input::MouseEvent(static_cast<int>(y * 10.0)));
    });

    glfwSetCharModsCallback(mWindow, [](GLFWwindow* w, unsigned c, int mods) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->sOnKeyEvent.emit(Input::KeyEvent(c, mods));
    });

  } else {
    Core::Logger::warning() << "Attempting to open an already opened window!" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::close() {
  if (mWindow != nullptr) {
    glfwDestroyWindow(mWindow);
    mWindow = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::shouldClose() const {
  return glfwWindowShouldClose(mWindow) != 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::update() {
  if (mWindow != nullptr) {
    glfwPollEvents();
    updateJoysticks();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::keyPressed(Input::Key key) const {
  if (mWindow == nullptr) {
    return false;
  }

  return glfwGetKey(mWindow, Core::Utils::enumCast(key)) == GLFW_PRESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::buttonPressed(Input::Button button) const {
  if (mWindow == nullptr) {
    return false;
  }

  if (button == Input::Button::eNone) {
    return false;
  }

  return glfwGetMouseButton(mWindow, Core::Utils::enumCast(button) - 1) == GLFW_PRESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Window::joyAxis(uint32_t joyStick, uint32_t axis) {
  if (joyStick >= Core::Utils::enumCast(Input::JoystickId::eJoystickNum) ||
      axis >= Core::Utils::enumCast(Input::JoystickAxisId::eJoystickAxisNum)) {
    return 0;
  }
  if (glfwJoystickPresent(joyStick) == 0) {
    return 0;
  }
  return mJoystickAxisCache[joyStick][axis];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::vec2 Window::getCursorPos() const {
  if (mWindow == nullptr) {
    return glm::vec2(0.f, 0.f);
  }
  double x, y;
  glfwGetCursorPos(mWindow, &x, &y);
  return glm::vec2(x, y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::present(BackedImagePtr const& image, vk::SemaphorePtr const& renderFinishedSemaphore,
    vk::FencePtr const& signalFence) {
  mSwapchain->present(image, renderFinishedSemaphore, signalFence);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::updateJoysticks() {
  float       changedThreshold(0.01f);
  const float minThreshold(0.15f);
  const float maxThreshold(0.9f);

  const int32_t joystickNum(Core::Utils::enumCast(Input::JoystickId::eJoystickNum));
  for (int32_t joy(0); joy < joystickNum; ++joy) {
    if (glfwJoystickPresent(joy) != 0) {
      auto    joyId(static_cast<Input::JoystickId>(joy));
      int32_t axesCount(0);
      auto    axesArray(glfwGetJoystickAxes(joy, &axesCount));

      for (int32_t axis(0); axis < axesCount; ++axis) {
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
        if (axis == 2 || axis == 5) {
          axisValue = (axisValue + 1) * 0.5;
        }
#endif

        int32_t sign(axisValue < 0.f ? -1 : 1);
        axisValue = std::abs(axisValue);

        axisValue = (axisValue - minThreshold) / (maxThreshold - minThreshold);
        axisValue = sign * std::min(1.f, std::max(0.f, axisValue));

        if (axisValue == 0.f || axisValue == 1.f || axisValue == -1.f) {
          changedThreshold = 0.f;
        }

        if (std::abs(axisValue - mJoystickAxisCache[joy][axis]) > changedThreshold) {
          auto axisId(static_cast<Input::JoystickAxisId>(axis));
          sOnJoystickAxisChanged.emit(joyId, axisId, axisValue);
          mJoystickAxisCache[joy][axis] = axisValue;
        }
      }

      int32_t buttonCount(0);
      auto    buttonArray(glfwGetJoystickButtons(joy, &buttonCount));
      for (int32_t button(0); button < buttonCount; ++button) {
        auto buttonId(static_cast<Input::JoystickButtonId>(button));
        auto buttonValue(static_cast<uint32_t>(buttonArray[button]));

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
