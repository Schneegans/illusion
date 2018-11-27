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

// ---------------------------------------------------------------------------------------- includes
#include "../Core/Property.hpp"
#include "../Input/KeyEvent.hpp"
#include "../Input/MouseEvent.hpp"
#include "Device.hpp"

struct GLFWwindow;
struct GLFWcursor;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Window {

 public:
  enum class Cursor { POINTER, IBEAM, CROSS, HAND, HRESIZE, VRESIZE };

  // ------------------------------------------------------------------------------------ properties
  Core::String           pTitle      = std::string("Illusion3D");
  Core::UVec2            pExtent     = glm::uvec2(640, 480);
  Core::IVec2            pPosition   = glm::ivec2(-1, -1);
  Core::Bool             pLockAspect = false;
  Core::Bool             pVsync      = false;
  Core::Bool             pHideCursor = false;
  Core::Bool             pFullscreen = false;
  Core::Property<Cursor> pCursor     = Cursor::POINTER;

  // --------------------------------------------------------------------------------------- signals
  Core::Signal<>                                                sOnClose;
  Core::Signal<bool>                                            sOnMinimize;
  Core::Signal<Input::KeyEvent>                                 sOnKeyEvent;
  Core::Signal<Input::MouseEvent>                               sOnMouseEvent;
  Core::Signal<Input::JoystickId, Input::JoystickAxisId, float> sOnJoystickAxisChanged;
  Core::Signal<Input::JoystickId, Input::JoystickButtonId>      sOnJoystickButtonPressed;
  Core::Signal<Input::JoystickId, Input::JoystickButtonId>      sOnJoystickButtonReleased;

  // -------------------------------------------------------------------------------- public methods
  Window(std::shared_ptr<Engine> const& engine, std::shared_ptr<Device> const& device);
  virtual ~Window();

  void open();
  void close();
  bool shouldClose() const;
  void processInput();

  bool      keyPressed(Input::Key key) const;
  bool      buttonPressed(Input::Button button) const;
  float     joyAxis(int joyStick, int axis);
  glm::vec2 getCursorPos() const;

  void present(
    std::shared_ptr<BackedImage> const&   image,
    std::shared_ptr<vk::Semaphore> const& renderFinishedSemaphore,
    std::shared_ptr<vk::Fence> const&     signalFence);

 private:
  // ------------------------------------------------------------------------------- private methods
  void updateJoysticks();

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Engine> mEngine;
  std::shared_ptr<Device> mDevice;
  GLFWwindow*             mWindow = nullptr;
  GLFWcursor*             mCursor = nullptr;

  std::shared_ptr<vk::SurfaceKHR> mSurface;
  std::shared_ptr<Swapchain>      mSwapchain;

  std::array<
    std::array<float, static_cast<int>(Input::JoystickAxisId::JOYSTICK_AXIS_NUM)>,
    static_cast<int>(Input::JoystickId::JOYSTICK_NUM)>
    mJoystickAxisCache;

  std::array<
    std::array<int, static_cast<int>(Input::JoystickButtonId::JOYSTICK_BUTTON_NUM)>,
    static_cast<int>(Input::JoystickId::JOYSTICK_NUM)>
    mJoystickButtonCache;

  glm::ivec2 mOrigSize = glm::ivec2(640, 480), mOrigPos = glm::ivec2(0, 0);
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_WINDOW_HPP
