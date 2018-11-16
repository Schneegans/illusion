////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/BitHash.hpp>
#include <Illusion/Core/FPSCounter.hpp>
#include <Illusion/Core/File.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Graphics/Context.hpp>
#include <Illusion/Graphics/DisplayPass.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/PhysicalDevice.hpp>
#include <Illusion/Graphics/Pipeline.hpp>
#include <Illusion/Graphics/ShaderModule.hpp>
#include <Illusion/Graphics/ShaderReflection.hpp>
#include <Illusion/Graphics/Texture.hpp>
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
  physicalDevice->printInfo();

  auto context   = std::make_shared<Illusion::Graphics::Context>(physicalDevice);
  auto window    = std::make_shared<Illusion::Graphics::Window>(engine, context);
  window->pVsync = false;
  window->open();

  std::vector<std::shared_ptr<Illusion::Graphics::ShaderModule>> modules;
  auto glsl = Illusion::Core::File<std::string>("data/shaders/PBRShader.frag").getContent();
  modules.push_back(
    std::make_shared<Illusion::Graphics::ShaderModule>(glsl, vk::ShaderStageFlagBits::eFragment));

  glsl = Illusion::Core::File<std::string>("data/shaders/PBRShader.vert").getContent();
  modules.push_back(
    std::make_shared<Illusion::Graphics::ShaderModule>(glsl, vk::ShaderStageFlagBits::eVertex));

  auto pipeline = std::make_shared<Illusion::Graphics::Pipeline>(context, modules);
  pipeline->getReflection()->printInfo();

  auto set0 = pipeline->allocateDescriptorSet(0);
  auto set1 = pipeline->allocateDescriptorSet(1);

  auto renderPass = window->getDisplayPass();
  renderPass->addAttachment(vk::Format::eD32Sfloat);

  renderPass->beforeFunc = [](vk::CommandBuffer const& cmd) {};
  renderPass->drawFunc   = [](vk::CommandBuffer const& cmd, uint32_t subPass) {};

  auto texture = Illusion::Graphics::Texture::createFromFile(context, "data/textures/box.dds");

  Illusion::Graphics::GraphicsState state;

  // std::unordered_map<Illusion::Graphics::GraphicsState, int> cache;

  auto hash = state.getHash();
  std::cout << hash.size() / 8 << std::endl;
  for (auto b : hash) {
    std::cout << (int)b << " ";
  }
  std::cout << std::endl;

  Illusion::Core::FPSCounter fpsCounter;
  fpsCounter.pFPS.onChange().connect([window](float fps) {
    std::stringstream title;
    title << appName << " (" << std::floor(fps) << " fps)";
    window->pTitle = title.str();
    return true;
  });

  while (!window->shouldClose()) {
    window->processInput();
    renderPass->render();
    fpsCounter.step();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return 0;
}
