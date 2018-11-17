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
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/PhysicalDevice.hpp>
#include <Illusion/Graphics/ShaderModule.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
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
  auto glsl = Illusion::Core::File<std::string>("data/shaders/TexturedQuad.frag").getContent();
  modules.push_back(std::make_shared<Illusion::Graphics::ShaderModule>(
    context, glsl, vk::ShaderStageFlagBits::eFragment));

  glsl = Illusion::Core::File<std::string>("data/shaders/TexturedQuad.vert").getContent();
  modules.push_back(std::make_shared<Illusion::Graphics::ShaderModule>(
    context, glsl, vk::ShaderStageFlagBits::eVertex));

  auto shader = std::make_shared<Illusion::Graphics::ShaderProgram>(context, modules);
  shader->getReflection()->printInfo();

  // auto set0 = shader->allocateDescriptorSet(0);

  auto renderPass = window->getDisplayPass();
  // renderPass->addAttachment(vk::Format::eD32Sfloat);
  renderPass->init();

  auto texture = Illusion::Graphics::Texture::createFromFile(context, "data/textures/box.dds");

  Illusion::Graphics::GraphicsState state;
  state.setShaderProgram(shader);

  Illusion::Graphics::GraphicsState::DepthStencilState depthStencilState;
  depthStencilState.mDepthTestEnable  = false;
  depthStencilState.mDepthWriteEnable = false;
  state.setDepthStencilState(depthStencilState);

  Illusion::Graphics::GraphicsState::ColorBlendState colorBlendState;
  colorBlendState.mAttachments.push_back(
    Illusion::Graphics::GraphicsState::ColorBlendState::AttachmentState());
  state.setColorBlendState(colorBlendState);

  window->pSize.onChange().connect([&state](glm::uvec2 const& size) {
    Illusion::Graphics::GraphicsState::ViewportState viewportState;
    viewportState.mViewports.push_back({glm::vec2(0), glm::vec2(size), 0.f, 1.f});
    viewportState.mScissors.push_back({glm::ivec2(0), size});
    state.setViewportState(viewportState);
    return true;
  });
  window->pSize.touch();

  struct PushConstants {
    glm::vec2 pos  = glm::vec2(0.2, 0.0);
    float     time = 0;
  } pushConstants;

  renderPass->drawFunc =
    [&state, &pushConstants](
      vk::CommandBuffer& cmd, Illusion::Graphics::RenderPass const& pass, uint32_t subPass) {
      auto pipeline = pass.createPipeline(state, subPass);
      pushConstants.time += 0.001;
      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
      cmd.pushConstants(
        *state.getShaderProgram()->getPipelineLayout(),
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(PushConstants),
        &pushConstants);
      cmd.draw(4, 1, 0, 0);
    };

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
