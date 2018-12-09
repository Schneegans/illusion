////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Logger.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/PipelineReflection.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto engine = Illusion::Graphics::Engine::create("Textured Quad Demo");
  auto device = Illusion::Graphics::Device::create(engine->getPhysicalDevice());
  auto window = Illusion::Graphics::Window::create(engine, device);

  auto texture = Illusion::Graphics::Texture::createFromFile(device, "data/textures/box.dds");
  auto shader  = Illusion::Graphics::ShaderProgram::createFromFiles(
    device, {"data/shaders/TexturedQuad.vert", "data/shaders/TexturedQuad.frag"});

  auto renderPass = Illusion::Graphics::RenderPass::create(device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  auto cmd = Illusion::Graphics::CommandBuffer::create(device);
  cmd->graphicsState().addBlendAttachment({});
  cmd->graphicsState().addViewport({glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f});
  cmd->graphicsState().addScissor({glm::ivec2(0), window->pExtent.get()});
  cmd->bindingState().setTexture(texture, 0, 0);
  cmd->begin();
  cmd->setShaderProgram(shader);
  cmd->beginRenderPass(renderPass);
  cmd->draw(4);
  cmd->endRenderPass();
  cmd->end();

  auto renderFinishedFence     = device->createFence();
  auto renderFinishedSemaphore = device->createSemaphore();

  window->open();
  while (!window->shouldClose()) {
    window->processInput();

    device->waitForFences(*renderFinishedFence, true, ~0);
    device->resetFences(*renderFinishedFence);

    cmd->submit({}, {}, {*renderFinishedSemaphore});

    window->present(
      renderPass->getFramebuffer()->getImages()[0], renderFinishedSemaphore, renderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}
