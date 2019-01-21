////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Instance.hpp"

#include "../Core/Logger.hpp"
#include "../Core/Utils.hpp"
#include "PhysicalDevice.hpp"
#include "VulkanPtr.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <sstream>

namespace Illusion::Graphics {

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_LUNARG_standard_validation"};

////////////////////////////////////////////////////////////////////////////////////////////////////

bool glfwInitialized = false;

////////////////////////////////////////////////////////////////////////////////////////////////////

VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                           messageType,
    const VkDebugUtilsMessengerCallbackDataEXT*               pCallbackData, void* /*pUserData*/) {

  // In the error message, Vulkan objects are referred to by a hex-string of their handle. In order
  // to improve the readability, we will try to replace each mention of a Vulkan object with the
  // actual name of the object.
  std::string message(pCallbackData->pMessage);

  for (uint32_t i(0); i < pCallbackData->objectCount; ++i) {
    if (pCallbackData->pObjects[i].pObjectName) {
      std::ostringstream address;
      address << (void const*)pCallbackData->pObjects[i].objectHandle;

      std::string hexHandle  = address.str();
      std::string objectName = "\"" + std::string(pCallbackData->pObjects[i].pObjectName) + "\"";

      Core::Utils::replaceString(message, hexHandle, objectName);
    }
  }

  if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    Core::Logger::trace() << message << std::endl;
  } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    Core::Logger::message() << message << std::endl;
  } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    Core::Logger::warning() << message << std::endl;
  } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    Core::Logger::error() << message << std::endl;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool checkValidationLayerSupport() {
  for (auto const& layer : VALIDATION_LAYERS) {
    bool layerFound = false;

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
  unsigned int glfwExtensionCount = 0;
  const char** glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions;
  for (unsigned int i = 0; i < glfwExtensionCount; ++i) {
    extensions.push_back(glfwExtensions[i]);
  }

  if (debugMode) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

Instance::Instance(std::string const& name, bool debugMode)
    : Core::NamedObject(name)
    , mDebugMode(debugMode)
    , mInstance(createInstance("Illusion", name))
    , mDebugCallback(createDebugCallback()) {

  Core::Logger::traceCreation("Instance", getName());

  for (auto const& vkPhysicalDevice : mInstance->enumeratePhysicalDevices()) {
    mPhysicalDevices.push_back(
        std::make_shared<PhysicalDevice>(*mInstance.get(), vkPhysicalDevice));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Instance::~Instance() {
  Core::Logger::traceDeletion("Instance", getName());
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

vk::SurfaceKHRPtr Instance::createSurface(std::string const& name, GLFWwindow* window) const {
  VkSurfaceKHR tmp;
  if (glfwCreateWindowSurface(*mInstance, window, nullptr, &tmp) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface!");
  }

  Core::Logger::traceCreation("vk::SurfaceKHR", name);

  // copying instance to keep reference counting up until the surface is destroyed
  auto instance = mInstance;
  return VulkanPtr::create(vk::SurfaceKHR(tmp), [instance, name](vk::SurfaceKHR* obj) {
    Core::Logger::traceDeletion("vk::SurfaceKHR", name);
    instance->destroySurfaceKHR(*obj);
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::InstancePtr Instance::getHandle() const {
  return mInstance;
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

  Core::Logger::traceCreation("vk::Instance", getName());
  auto name = getName();
  return VulkanPtr::create(vk::createInstance(info), [name](vk::Instance* obj) {
    Core::Logger::traceDeletion("vk::Instance", name);
    obj->destroy();
    delete obj;
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DebugUtilsMessengerEXTPtr Instance::createDebugCallback() const {
  if (!mDebugMode) {
    return nullptr;
  }

  auto createCallback{
      (PFN_vkCreateDebugUtilsMessengerEXT)mInstance->getProcAddr("vkCreateDebugUtilsMessengerEXT")};

  vk::DebugUtilsMessengerCreateInfoEXT info;
  info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  info.pfnUserCallback = debugCallback;

  VkDebugUtilsMessengerEXT tmp;
  if (createCallback(*mInstance, (VkDebugUtilsMessengerCreateInfoEXT*)&info, nullptr, &tmp)) {
    throw std::runtime_error("Failed to set up debug callback!");
  }

  auto name = "DebugCallback for " + getName();
  Core::Logger::traceCreation("vk::DebugUtilsMessengerEXT", name);
  auto instance = mInstance;
  return VulkanPtr::create(
      vk::DebugUtilsMessengerEXT(tmp), [instance, name](vk::DebugUtilsMessengerEXT* obj) {
        auto destroyCallback = (PFN_vkDestroyDebugUtilsMessengerEXT)instance->getProcAddr(
            "vkDestroyDebugUtilsMessengerEXT");
        Core::Logger::traceDeletion("vk::DebugUtilsMessengerEXT", name);
        destroyCallback(*instance, *obj, nullptr);
        delete obj;
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
