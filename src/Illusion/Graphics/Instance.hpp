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
////////////////////////////////////////////////////////////////////////////////////////////////////

class Instance {

 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static InstancePtr create(Args&&... args) {
    return std::make_shared<Instance>(args...);
  };

  explicit Instance(std::string const& appName, bool debugMode = true);
  virtual ~Instance();

  PhysicalDevicePtr getPhysicalDevice(
      std::vector<std::string> const& extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}) const;

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
