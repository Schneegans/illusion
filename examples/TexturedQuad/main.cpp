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
#include <Illusion/Graphics/DescriptorSet.hpp>
#include <Illusion/Graphics/DescriptorSetCache.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/PipelineReflection.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto engine = std::make_shared<Illusion::Graphics::Engine>("Textured Quad Demo");
  auto device = std::make_shared<Illusion::Graphics::Device>(engine->getPhysicalDevice());
  auto window = std::make_shared<Illusion::Graphics::Window>(engine, device);

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    device,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/TexturedQuad.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/TexturedQuad.frag"}});

  auto texture = Illusion::Graphics::Texture::createFromFile(device, "data/textures/box.dds");

  Illusion::Graphics::GraphicsState state;
  state.setShaderProgram(shader);
  state.addBlendAttachment({});
  state.addViewport({glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f});
  state.addScissor({glm::ivec2(0), window->pExtent.get()});

  auto renderPass = std::make_shared<Illusion::Graphics::RenderPass>(device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  auto commandBuffer           = device->allocateGraphicsCommandBuffer();
  auto renderFinishedFence     = device->createFence({vk::FenceCreateFlagBits::eSignaled});
  auto renderFinishedSemaphore = device->createSemaphore({});
  auto descriptorSetCache      = std::make_shared<Illusion::Graphics::DescriptorSetCache>(device);
  auto descriptorSet =
    descriptorSetCache->acquireHandle(shader->getDescriptorSetReflections().at(0));
  descriptorSet->bindCombinedImageSampler(texture, 0);

  window->open();
  while (!window->shouldClose()) {
    window->processInput();

    device->waitForFences(*renderFinishedFence, true, ~0);
    device->resetFences(*renderFinishedFence);

    commandBuffer->reset({});
    commandBuffer->begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    renderPass->begin(commandBuffer);

    auto pipeline = renderPass->getPipelineHandle(state);
    commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    commandBuffer->bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      *state.getShaderProgram()->getReflection()->getLayout(),
      descriptorSet->getSet(),
      *descriptorSet,
      nullptr);
    commandBuffer->draw(4, 1, 0, 0);
    renderPass->end(commandBuffer);

    commandBuffer->end();

    device->submit({*commandBuffer}, {}, {}, {*renderFinishedSemaphore});

    window->present(
      renderPass->getFramebuffer()->getImages()[0], renderFinishedSemaphore, renderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  device->waitIdle();

  return 0;
}
