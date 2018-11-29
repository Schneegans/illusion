////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_ENGINE_HPP
#define ILLUSION_GRAPHICS_ENGINE_HPP

#include "fwd.hpp"

struct GLFWwindow;

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class Engine {

 public:
  Engine(std::string const& appName, bool debugMode = true);
  virtual ~Engine();

  std::shared_ptr<PhysicalDevice> getPhysicalDevice(
    bool                            graphics   = true,
    bool                            compute    = true,
    bool                            present    = true,
    std::vector<std::string> const& extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}) const;

  std::shared_ptr<vk::SurfaceKHR> createSurface(GLFWwindow* window) const;

 private:
  std::shared_ptr<vk::Instance> createInstance(
    std::string const& engine, std::string const& app) const;
  std::shared_ptr<vk::DebugReportCallbackEXT> createDebugCallback() const;

  bool mDebugMode{false};

  std::shared_ptr<vk::Instance>                mInstance;
  std::shared_ptr<vk::DebugReportCallbackEXT>  mDebugCallback;
  std::vector<std::shared_ptr<PhysicalDevice>> mPhysicalDevices;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_ENGINE_HPP
