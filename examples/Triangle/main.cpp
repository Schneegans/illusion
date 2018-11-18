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
#include <Illusion/Graphics/DisplayPass.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

int main(int argc, char* argv[]) {
  auto engine  = std::make_shared<Illusion::Graphics::Engine>("Triangle Demo");
  auto context = std::make_shared<Illusion::Graphics::Context>(engine->getPhysicalDevice());
  auto window  = std::make_shared<Illusion::Graphics::Window>(engine, context);
  window->open();

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    context,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/Triangle.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/Triangle.frag"}});

  Illusion::Graphics::GraphicsState state;
  state.setShaderProgram(shader);

  Illusion::Graphics::GraphicsState::ColorBlendState colorBlendState;
  colorBlendState.mAttachments.resize(1);
  state.setColorBlendState(colorBlendState);

  Illusion::Graphics::GraphicsState::ViewportState viewportState;
  viewportState.mViewports.push_back({glm::vec2(0), glm::vec2(window->pSize.get()), 0.f, 1.f});
  viewportState.mScissors.push_back({glm::ivec2(0), window->pSize.get()});
  state.setViewportState(viewportState);

  window->pSize.onChange().connect([&state](glm::uvec2 const& size) {
    auto viewportState                  = state.getViewportState();
    viewportState.mViewports[0].mExtend = size;
    viewportState.mScissors[0].mExtend  = size;
    state.setViewportState(viewportState);
    return true;
  });

  auto renderPass      = window->getDisplayPass();
  renderPass->drawFunc = [&state](
                           std::shared_ptr<Illusion::Graphics::CommandBuffer> cmd,
                           Illusion::Graphics::RenderPass const&              pass,
                           uint32_t                                           subPass) {
    auto pipeline = pass.getPipelineHandle(state, subPass);
    cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmd->draw(3, 1, 0, 0);
  };

  while (!window->shouldClose()) {
    window->processInput();
    renderPass->render();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return 0;
}
