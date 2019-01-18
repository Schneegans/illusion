////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_INSTANCE_HPP
#define ILLUSION_GRAPHICS_INSTANCE_HPP

#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

struct GLFWwindow;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Instance is the first object you have to create when you want to do Vulkan rendering. Its  //
// constructor requires nothing more than a name to identify your application.                    //
// It can then be used to get a PhysicalDevice which is required to create a Device. Once you     //
// have a Device, you can create all othe Vulkan resources.                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Instance : public Core::StaticCreate<Instance> {

 public:
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
