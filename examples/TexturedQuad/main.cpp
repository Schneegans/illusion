////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/DescriptorSet.hpp>
#include <Illusion/Graphics/DisplayPass.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

int main(int argc, char* argv[]) {
  auto engine  = std::make_shared<Illusion::Graphics::Engine>("Textured Quad Demo");
  auto context = std::make_shared<Illusion::Graphics::Context>(engine->getPhysicalDevice());
  auto window  = std::make_shared<Illusion::Graphics::Window>(engine, context);
  window->open();

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    context,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/TexturedQuad.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/TexturedQuad.frag"}});

  auto texture = Illusion::Graphics::Texture::createFromFile(context, "data/textures/box.dds");

  auto descriptorSet = shader->allocateDescriptorSet();
  descriptorSet->bindCombinedImageSampler(texture, 0);

  Illusion::Graphics::GraphicsState state;
  state.setShaderProgram(shader);
  state.addBlendAttachment({});
  state.addViewport({glm::vec2(0), glm::vec2(window->pSize.get()), 0.f, 1.f});
  state.addScissor({glm::ivec2(0), window->pSize.get()});

  window->pSize.onChange().connect([&state](glm::uvec2 const& size) {
    auto viewports       = state.getViewports();
    viewports[0].mExtend = size;
    viewports[0].mExtend = size;
    state.setViewports(viewports);
    return true;
  });

  auto renderPass      = window->getDisplayPass();
  renderPass->drawFunc = [&state, &descriptorSet](
                           std::shared_ptr<Illusion::Graphics::CommandBuffer> cmd,
                           Illusion::Graphics::RenderPass const&              pass,
                           uint32_t                                           subPass) {
    auto pipeline = pass.getPipelineHandle(state, subPass);
    cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmd->bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      *state.getShaderProgram()->getPipelineLayout(),
      descriptorSet->getSet(),
      *descriptorSet,
      nullptr);
    cmd->draw(4, 1, 0, 0);
  };

  while (!window->shouldClose()) {
    window->processInput();
    renderPass->render();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return 0;
}
