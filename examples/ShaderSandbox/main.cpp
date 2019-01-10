////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/CommandLineOptions.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

struct PushConstants {
  float iTime;
  float iAspect;
};

int main(int argc, char* argv[]) {

  std::string shaderFile = "data/shaders/Sandbox.frag";
  bool        printHelp  = false;

  Illusion::Core::CommandLineOptions args("Renders a full screen texture.");
  args.addOption({"-h", "--help"}, &printHelp, "Print this help");
  args.addOption({"-t", "--trace"}, &Illusion::Core::Logger::enableTrace, "Print trace output");
  args.addOption({"-s", "--shader"}, &shaderFile,
      "The fragment shader file to use. This defaults to data/shaders/Sandbox.frag");
  args.parse(argc, argv);

  if (printHelp) {
    args.printHelp();
    return 0;
  }

  auto engine = Illusion::Graphics::Engine::create("Shader Sandbox");
  auto device = Illusion::Graphics::Device::create(engine->getPhysicalDevice());
  auto window = Illusion::Graphics::Window::create(engine, device);

  auto shader =
      Illusion::Graphics::Shader::createFromFiles(device, {"data/shaders/Quad.vert", shaderFile});

  auto renderPass = Illusion::Graphics::RenderPass::create(device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  auto cmd = Illusion::Graphics::CommandBuffer::create(device);
  cmd->graphicsState().addBlendAttachment({});
  cmd->graphicsState().addViewport({glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f});

  auto renderFinishedFence     = device->createFence();
  auto renderFinishedSemaphore = device->createSemaphore();

  Illusion::Core::Timer timer;

  window->open();
  while (!window->shouldClose()) {
    window->processInput();

    device->waitForFences(*renderFinishedFence);
    device->resetFences(*renderFinishedFence);

    renderPass->setExtent(window->pExtent.get());

    glm::vec2 windowSize = window->pExtent.get();
    cmd->graphicsState().setViewports({{glm::vec2(0), windowSize, 0.f, 1.f}});

    cmd->reset();
    cmd->begin();
    cmd->setShader(shader);
    cmd->pushConstants(PushConstants{float(timer.getElapsed()), windowSize.x / windowSize.y});
    cmd->beginRenderPass(renderPass);
    cmd->draw(4);
    cmd->endRenderPass();
    cmd->end();
    cmd->submit({}, {}, {*renderFinishedSemaphore});

    window->present(
        renderPass->getFramebuffer()->getImages()[0], renderFinishedSemaphore, renderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}
