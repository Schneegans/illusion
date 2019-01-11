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
// The window is the place were you can draw stuff to. It is implemented using glfw. Its public   //
// interface makes extensive use of Signals and Properties you can connect to. All Signals will   //
// be emitted from the update() method.                                                           //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Window {

 public:
  // properties ------------------------------------------------------------------------------------

  // This string is shown in the title of the window.
  Core::String pTitle = std::string("Illusion");

  // The current size of the window. After open() has been called, setting this value has no effect.
  Core::UVec2 pExtent = glm::uvec2(640, 480);

  // When this is true, the user won't be able to change the cureent spect ration of the Window.
  Core::Bool pLockAspect = false;

  // Setting this value triggers a re-creation of the Swapchain.
  Core::Bool pVsync = false;

  // Shows or hides the mouse cursor when it hovers the Window.
  Core::Bool pHideCursor = false;

  // When changed after open() has been called, this will trigger a re-creation of the Swapchain.
  Core::Bool pFullscreen = false;

  // Set the mouse pointer to one of the available shapes.
  enum class Cursor { ePointer, eIBeam, eCross, eHand, eHResize, eVResize };
  Core::Property<Cursor> pCursor = Cursor::ePointer;

  // signals ---------------------------------------------------------------------------------------

  // You can use these Signals to react to input events.
  Core::Signal<Input::KeyEvent>                                 sOnKeyEvent;
  Core::Signal<Input::MouseEvent>                               sOnMouseEvent;
  Core::Signal<Input::JoystickId, Input::JoystickAxisId, float> sOnJoystickAxisChanged;
  Core::Signal<Input::JoystickId, Input::JoystickButtonId>      sOnJoystickButtonPressed;
  Core::Signal<Input::JoystickId, Input::JoystickButtonId>      sOnJoystickButtonReleased;

  // Emitted when the Window is closed. This happens either when the close() gets called.
  Core::Signal<> sOnClose;

  // methods ---------------------------------------------------------------------------------------

  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static WindowPtr create(Args&&... args) {
    return std::make_shared<Window>(args...);
  };

  // Once you created an Instance and a Device, you can start creating Windows.
  Window(InstancePtr const& instance, DevicePtr const& device);
  virtual ~Window();

  // You should set up the Properties above (especially the pExtent) before calling this method
  void open();

  // Call this once a frame. This method will actually lead to the emission of the Signals of the
  // Window if a corresponding event occurred.
  void update();

  // You should call this method regularly as well. It will return true when the user clicked the
  // close button in the title bar or pressed Alt+F4 or something similar. Normally you should call
  // close() (or the destructor) in this case.
  bool shouldClose() const;

  // Closes the Window. It can be reopened later if you wish.
  void close();

  // Returns true if the given key is currently held down. This should only be used for continuous
  // input - if you are interested in key-press events you should rather use the Signals of this
  // class.
  bool keyPressed(Input::Key key) const;

  // Returns true if the given mouse buttons is currently held down. This should only be used for
  // continuous input - if you are interested in button-press events you should rather use the
  // Signals of this class.
  bool buttonPressed(Input::Button button) const;

  // Returns the position of the given joystick axis.
  float joyAxis(int joyStick, int axis);

  // Returns the current mouse pointer position.
  glm::vec2 getCursorPos() const;

  // This is forwarded to the internal Swapchain. The given image will be blitted to one of the
  // swapchain images. The operation will wait for the given semaphore and will signal the given
  // fence once it finishes.
  void present(BackedImagePtr const& image, vk::SemaphorePtr const& renderFinishedSemaphore,
      vk::FencePtr const& signalFence);

 private:
  void updateJoysticks();

  InstancePtr mInstance;
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

  // Stores position and size for restoring the window state after toggling full screen mode.
  glm::ivec2 mOrigSize = glm::ivec2(640, 480), mOrigPos = glm::ivec2(0, 0);
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_WINDOW_HPP
