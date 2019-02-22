////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_INSTANCE_HPP
#define ILLUSION_GRAPHICS_INSTANCE_HPP

#include "../Core/NamedObject.hpp"
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

class Instance : public Core::StaticCreate<Instance>, public Core::NamedObject {

 public:
  // When debugMode is set to true, validation layers will be loaded. This can throw a
  // std::runtime_error for various reasons.
  explicit Instance(std::string const& name, bool debugMode = true);
  virtual ~Instance();

  // Tries to find a physical device which supports the given extensions. This will throw a
  // std::runtime_error when there is no suitable Vulkan device.
  PhysicalDeviceConstPtr getPhysicalDevice(
      std::vector<std::string> const& extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}) const;

  // This is used by the Window class. This will throw a std::runtime_error when glfw failed to
  // create a Vulkan surface.
  vk::SurfaceKHRPtr createSurface(std::string const& name, GLFWwindow* window) const;

  // Access to the underlying vk::instance
  vk::InstancePtr getHandle() const;

 private:
  vk::InstancePtr createInstance(std::string const& engine, std::string const& app) const;
  vk::DebugUtilsMessengerEXTPtr createDebugCallback() const;

  bool mDebugMode = false;

  vk::InstancePtr                     mInstance;
  vk::DebugUtilsMessengerEXTPtr       mDebugCallback;
  std::vector<PhysicalDeviceConstPtr> mPhysicalDevices;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_INSTANCE_HPP
