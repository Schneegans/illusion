////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Instance.hpp"

#include "../Core/Logger.hpp"
#include "PhysicalDevice.hpp"
#include "VulkanPtr.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <sstream>

namespace Illusion::Graphics {

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<const char*> VALIDATION_LAYERS{"VK_LAYER_LUNARG_standard_validation"};

////////////////////////////////////////////////////////////////////////////////////////////////////

bool glfwInitialized{false};

////////////////////////////////////////////////////////////////////////////////////////////////////

VkBool32 messageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
    uint64_t /*object*/, size_t /*location*/, int32_t code, const char* layer, const char* message,
    void* /*userData*/) {

  std::stringstream buf;
  buf << "[" << layer << "] " << message << " (code: " << code << ")" << std::endl;

  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    ILLUSION_ERROR << buf.str();
  } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    ILLUSION_WARNING << buf.str();
  } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    ILLUSION_TRACE << buf.str();
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool checkValidationLayerSupport() {
  for (auto const& layer : VALIDATION_LAYERS) {
    bool layerFound{false};

    for (auto const& property : vk::enumerateInstanceLayerProperties()) {
      if (std::strcmp(layer, property.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<const char*> getRequiredInstanceExtensions(bool debugMode) {
  unsigned int glfwExtensionCount{0};
  const char** glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};

  std::vector<const char*> extensions;
  for (unsigned int i = 0; i < glfwExtensionCount; ++i) {
    extensions.push_back(glfwExtensions[i]);
  }

  if (debugMode) {
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  return extensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

Instance::Instance(std::string const& app, bool debugMode)
    : mDebugMode(debugMode)
    , mInstance(createInstance("Illusion", app))
    , mDebugCallback(createDebugCallback()) {

  ILLUSION_TRACE << "Creating Instance." << std::endl;

  for (auto const& vkPhysicalDevice : mInstance->enumeratePhysicalDevices()) {
    mPhysicalDevices.push_back(
        std::make_shared<PhysicalDevice>(*mInstance.get(), vkPhysicalDevice));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Instance::~Instance() {
  ILLUSION_TRACE << "Deleting Instance." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PhysicalDevicePtr Instance::getPhysicalDevice(std::vector<std::string> const& extensions) const {

  // loop through physical devices and choose a suitable one
  for (auto const& physicalDevice : mPhysicalDevices) {

    // check whether all required extensions are supported
    auto availableExtensions = physicalDevice->enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (auto const& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    if (!requiredExtensions.empty()) {
      continue;
    }

    // all required extensions are supported - take this device!
    return physicalDevice;
  }

  throw std::runtime_error("Failed to find a suitable vulkan device!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SurfaceKHRPtr Instance::createSurface(GLFWwindow* window) const {
  VkSurfaceKHR tmp;
  if (glfwCreateWindowSurface(*mInstance, window, nullptr, &tmp) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface!");
  }

  ILLUSION_TRACE << "Creating vk::SurfaceKHR." << std::endl;

  // copying instance to keep reference counting up until the surface is destroyed
  auto instance{mInstance};
  return VulkanPtr::create(vk::SurfaceKHR(tmp), [instance](vk::SurfaceKHR* obj) {
    ILLUSION_TRACE << "Deleting vk::SurfaceKHR." << std::endl;
    instance->destroySurfaceKHR(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::InstancePtr Instance::createInstance(std::string const& engine, std::string const& app) const {

  if (!glfwInitialized) {
    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwSetErrorCallback([](int /*error*/, const char* description) {
      throw std::runtime_error("GLFW: " + std::string(description));
    });

    glfwInitialized = true;
  }

  if (mDebugMode && !checkValidationLayerSupport()) {
    throw std::runtime_error("Requested validation layers are not available!");
  }

  // app info
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName   = app.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName        = engine.c_str();
  appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion         = VK_API_VERSION_1_0;

  // find required extensions
  auto extensions(getRequiredInstanceExtensions(mDebugMode));

  // create instance
  vk::InstanceCreateInfo info;
  info.pApplicationInfo        = &appInfo;
  info.enabledExtensionCount   = static_cast<int32_t>(extensions.size());
  info.ppEnabledExtensionNames = extensions.data();

  if (mDebugMode) {
    info.enabledLayerCount   = static_cast<int32_t>(VALIDATION_LAYERS.size());
    info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  } else {
    info.enabledLayerCount = 0;
  }

  ILLUSION_TRACE << "Creating vk::Instance." << std::endl;
  return VulkanPtr::create(vk::createInstance(info), [](vk::Instance* obj) {
    ILLUSION_TRACE << "Deleting vk::Instance." << std::endl;
    obj->destroy();
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DebugReportCallbackEXTPtr Instance::createDebugCallback() const {
  if (!mDebugMode) {
    return nullptr;
  }

  auto createCallback{
      (PFN_vkCreateDebugReportCallbackEXT)mInstance->getProcAddr("vkCreateDebugReportCallbackEXT")};

  vk::DebugReportCallbackCreateInfoEXT info;
  info.flags = vk::DebugReportFlagBitsEXT::eInformation | vk::DebugReportFlagBitsEXT::eWarning |
               vk::DebugReportFlagBitsEXT::ePerformanceWarning |
               vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eDebug;
  info.pfnCallback = messageCallback;

  VkDebugReportCallbackEXT tmp;
  if (createCallback(*mInstance, (VkDebugReportCallbackCreateInfoEXT*)&info, nullptr, &tmp)) {
    throw std::runtime_error("Failed to set up debug callback!");
  }

  ILLUSION_TRACE << "Creating vk::DebugReportCallbackEXT." << std::endl;
  auto instance{mInstance};
  return VulkanPtr::create(
      vk::DebugReportCallbackEXT(tmp), [instance](vk::DebugReportCallbackEXT* obj) {
        auto destroyCallback = (PFN_vkDestroyDebugReportCallbackEXT)instance->getProcAddr(
            "vkDestroyDebugReportCallbackEXT");
        ILLUSION_TRACE << "Deleting vk::DebugReportCallbackEXT." << std::endl;
        destroyCallback(*instance, *obj, nullptr);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
