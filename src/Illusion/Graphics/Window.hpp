////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_WINDOW_HPP
#define ILLUSION_GRAPHICS_WINDOW_HPP

#include "../Core/Property.hpp"
#include "../Input/KeyEvent.hpp"
#include "../Input/MouseEvent.hpp"
#include "Device.hpp"

struct GLFWwindow;
struct GLFWcursor;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Window {

 public:
  enum class Cursor { ePointer, eIBeam, eCross, eHand, eHResize, eVResize };

  Core::String           pTitle      = std::string("Illusion3D");
  Core::UVec2            pExtent     = glm::uvec2(640, 480);
  Core::IVec2            pPosition   = glm::ivec2(-1, -1);
  Core::Bool             pLockAspect = false;
  Core::Bool             pVsync      = false;
  Core::Bool             pHideCursor = false;
  Core::Bool             pFullscreen = false;
  Core::Property<Cursor> pCursor     = Cursor::ePointer;

  Core::Signal<>                                                sOnClose;
  Core::Signal<bool>                                            sOnMinimize;
  Core::Signal<Input::KeyEvent>                                 sOnKeyEvent;
  Core::Signal<Input::MouseEvent>                               sOnMouseEvent;
  Core::Signal<Input::JoystickId, Input::JoystickAxisId, float> sOnJoystickAxisChanged;
  Core::Signal<Input::JoystickId, Input::JoystickButtonId>      sOnJoystickButtonPressed;
  Core::Signal<Input::JoystickId, Input::JoystickButtonId>      sOnJoystickButtonReleased;

  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static WindowPtr create(Args&&... args) {
    return std::make_shared<Window>(args...);
  };

  Window(EnginePtr const& engine, DevicePtr const& device);
  virtual ~Window();

  void open();
  void close();
  bool shouldClose() const;
  void processInput();

  bool      keyPressed(Input::Key key) const;
  bool      buttonPressed(Input::Button button) const;
  float     joyAxis(int joyStick, int axis);
  glm::vec2 getCursorPos() const;

  void present(BackedImagePtr const& image, vk::SemaphorePtr const& renderFinishedSemaphore,
    vk::FencePtr const& signalFence);

 private:
  void updateJoysticks();

  EnginePtr   mEngine;
  DevicePtr   mDevice;
  GLFWwindow* mWindow = nullptr;
  GLFWcursor* mCursor = nullptr;

  vk::SurfaceKHRPtr mSurface;
  SwapchainPtr      mSwapchain;

  std::array<std::array<float, Core::enumCast(Input::JoystickAxisId::eJoystickAxisNum)>,
    Core::enumCast(Input::JoystickId::eJoystickNum)>
    mJoystickAxisCache;

  std::array<std::array<int, Core::enumCast(Input::JoystickButtonId::eJoystickButtonNum)>,
    Core::enumCast(Input::JoystickId::eJoystickNum)>
    mJoystickButtonCache;

  glm::ivec2 mOrigSize = glm::ivec2(640, 480), mOrigPos = glm::ivec2(0, 0);
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_WINDOW_HPP
