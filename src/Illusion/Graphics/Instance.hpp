////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_INSTANCE_HPP
#define ILLUSION_GRAPHICS_INSTANCE_HPP

#include "fwd.hpp"

struct GLFWwindow;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Instance is the first object you have to create when you want to do Vulkan rendering. Its  //
// constructor requires nothing more than a name to identify your application.                    //
// It can then be used to get a PhysicalDevice which is required to create a Device. Once you     //
// have a Device, you can create all othe Vulkan resources.                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Instance {

 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static InstancePtr create(Args&&... args) {
    return std::make_shared<Instance>(args...);
  };

  // When debugMode is set to true, validation layers will be loaded.
  explicit Instance(std::string const& appName, bool debugMode = true);
  virtual ~Instance();

  // Tries to find a physical device which supports the given extensions.
  PhysicalDevicePtr getPhysicalDevice(
      std::vector<std::string> const& extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}) const;

  // This is used by the Window class.
  vk::SurfaceKHRPtr createSurface(GLFWwindow* window) const;

 private:
  vk::InstancePtr createInstance(std::string const& engine, std::string const& app) const;
  vk::DebugReportCallbackEXTPtr createDebugCallback() const;

  bool mDebugMode = false;

  vk::InstancePtr                mInstance;
  vk::DebugReportCallbackEXTPtr  mDebugCallback;
  std::vector<PhysicalDevicePtr> mPhysicalDevices;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_INSTANCE_HPP
