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
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

int main(int argc, char* argv[]) {

  bool useHLSL   = false;
  bool printHelp = false;

  Illusion::Core::CommandLineOptions args("Renders a full screen texture.");
  args.addOption({"-h", "--help"}, &printHelp, "Print this help");
  args.addOption({"--hlsl"}, &useHLSL, "Use HLSL shaders instead of GLSL shaders");
  args.addOption({"-t", "--trace"}, &Illusion::Core::Logger::enableTrace, "Print trace output");
  args.parse(argc, argv);

  if (printHelp) {
    args.printHelp();
    return 0;
  }

  auto instance = Illusion::Graphics::Instance::create("Textured Quad Demo");
  auto device   = Illusion::Graphics::Device::create(instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create(instance, device);

  auto texture = Illusion::Graphics::Texture::createFromFile(device, "data/textures/box.dds");
  auto shader  = Illusion::Graphics::Shader::createFromFiles(device,
      useHLSL
          ? std::vector<std::string>{"data/shaders/Quad.vs", "data/shaders/TexturedQuad.ps"}
          : std::vector<std::string>{"data/shaders/Quad.vert", "data/shaders/TexturedQuad.frag"});

  auto renderPass = Illusion::Graphics::RenderPass::create(device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  auto cmd = Illusion::Graphics::CommandBuffer::create(device);
  cmd->graphicsState().addBlendAttachment({});
  cmd->graphicsState().addViewport({glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f});
  cmd->bindingState().setTexture(texture, 0, 0);
  cmd->begin();
  cmd->setShader(shader);
  cmd->beginRenderPass(renderPass);
  cmd->draw(4);
  cmd->endRenderPass();
  cmd->end();

  auto renderFinishedFence     = device->createFence();
  auto renderFinishedSemaphore = device->createSemaphore();

  window->open();
  while (!window->shouldClose()) {
    window->update();

    device->waitForFences(*renderFinishedFence);
    device->resetFences(*renderFinishedFence);

    cmd->submit({}, {}, {*renderFinishedSemaphore});

    window->present(
        renderPass->getFramebuffer()->getImages()[0], renderFinishedSemaphore, renderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}
