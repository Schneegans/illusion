////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/FPSCounter.hpp>
#include <Illusion/Core/File.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Graphics/Context.hpp>
#include <Illusion/Graphics/DisplayPass.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/PhysicalDevice.hpp>
#include <Illusion/Graphics/ShaderModule.hpp>
#include <Illusion/Graphics/ShaderReflection.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <iostream>
#include <sstream>
#include <thread>

std::string appName = "SimpleWindow";

int main(int argc, char* argv[]) {

#ifdef NDEBUG
  Illusion::Core::Logger::enableDebug = false;
  Illusion::Core::Logger::enableTrace = false;
  auto engine = std::make_shared<Illusion::Graphics::Engine>(appName, false);
#else
  Illusion::Core::Logger::enableDebug = true;
  Illusion::Core::Logger::enableTrace = true;
  auto engine                         = std::make_shared<Illusion::Graphics::Engine>(appName, true);
#endif

  auto physicalDevice = engine->getPhysicalDevice();
  auto context        = std::make_shared<Illusion::Graphics::Context>(physicalDevice);
  auto window         = std::make_shared<Illusion::Graphics::Window>(engine, context);
  window->pVsync      = false;
  window->open();

  auto glsl = Illusion::Core::File<std::string>("data/shaders/PBRShader.frag").getContent();
  auto frag =
    std::make_shared<Illusion::Graphics::ShaderModule>(glsl, vk::ShaderStageFlagBits::eFragment);

  glsl = Illusion::Core::File<std::string>("data/shaders/PBRShader.vert").getContent();
  auto vert =
    std::make_shared<Illusion::Graphics::ShaderModule>(glsl, vk::ShaderStageFlagBits::eVertex);

  auto reflection = std::make_shared<Illusion::Graphics::ShaderReflection>();
  reflection->addResources(frag->getResources());
  reflection->addResources(vert->getResources());
  reflection->printInfo();

  Illusion::Core::FPSCounter fpsCounter;
  fpsCounter.pFPS.onChange().connect([window](float fps) {
    std::stringstream title;
    title << appName << " (" << std::floor(fps) << " fps)";
    window->pTitle = title.str();
    return true;
  });

  while (!window->shouldClose()) {
    window->processInput();
    window->getDisplayPass()->render();
    fpsCounter.step();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return 0;
}
