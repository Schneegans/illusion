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

#include <glm/gtc/matrix_transform.hpp>
#include <thread>

// clang-format off
const std::array<glm::vec3, 26> POSITIONS = {
  glm::vec3( 1, -1, 1),  glm::vec3(-1, -1, -1), glm::vec3( 1, -1, -1), glm::vec3(-1,  1, -1),
  glm::vec3( 1,  1,  1), glm::vec3( 1,  1, -1), glm::vec3( 1,  1, -1), glm::vec3( 1, -1,  1),
  glm::vec3( 1, -1, -1), glm::vec3( 1,  1,  1), glm::vec3(-1, -1,  1), glm::vec3( 1, -1,  1),
  glm::vec3(-1, -1,  1), glm::vec3(-1,  1, -1), glm::vec3(-1, -1, -1), glm::vec3( 1, -1, -1),
  glm::vec3(-1,  1, -1), glm::vec3( 1,  1, -1), glm::vec3(-1, -1,  1), glm::vec3(-1,  1,  1),
  glm::vec3( 1,  1, -1), glm::vec3( 1,  1,  1), glm::vec3( 1, -1,  1), glm::vec3(-1,  1,  1),
  glm::vec3(-1,  1,  1), glm::vec3(-1, -1, -1)};

const std::array<glm::vec3, 26> NORMALS = {
  glm::vec3( 0, -1,  0), glm::vec3( 0, -1,  0), glm::vec3( 0, -1,  0), glm::vec3( 0,  1,  0),
  glm::vec3( 0,  1,  0), glm::vec3( 0,  1,  0), glm::vec3( 1,  0,  0), glm::vec3( 1,  0,  0),
  glm::vec3( 1,  0,  0), glm::vec3( 0,  0,  1), glm::vec3( 0,  0,  1), glm::vec3( 0,  0,  1),
  glm::vec3(-1,  0,  0), glm::vec3(-1,  0,  0), glm::vec3(-1,  0,  0), glm::vec3( 0,  0, -1),
  glm::vec3( 0,  0, -1), glm::vec3( 0,  0, -1), glm::vec3( 0, -1,  0), glm::vec3( 0,  1,  0),
  glm::vec3( 1,  0,  0), glm::vec3( 1,  0,  0), glm::vec3( 1,  0,  0), glm::vec3( 0,  0,  1),
  glm::vec3(-1,  0,  0), glm::vec3( 0,  0, -1)};

const std::array<glm::vec2, 26> TEXCOORDS = {
  glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1),
  glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 0), 
  glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(0, 1), 
  glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(1, 1), 
  glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), 
  glm::vec2(1, 1)};

const std::array<uint32_t, 36> INDICES = {
  0, 1,  2, 3, 4,  5, 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
  0, 18, 1, 3, 19, 4, 20, 21, 22, 9, 23, 10, 12, 24, 13, 15, 25, 16,
};
// clang-format on

struct CameraUniforms {
  glm::mat4 projection;
};

struct PushConstants {
  glm::mat4 modelView;
};

int main(int argc, char* argv[]) {
  auto engine  = std::make_shared<Illusion::Graphics::Engine>("Textured Cube Demo");
  auto context = std::make_shared<Illusion::Graphics::Context>(engine->getPhysicalDevice());
  auto window  = std::make_shared<Illusion::Graphics::Window>(engine, context);
  window->open();

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    context,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/TexturedCube.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/TexturedCube.frag"}});

  auto cameraUniformDescriptorSet = shader->allocateDescriptorSet(0);
  auto cameraUniformBuffer        = context->createUniformBuffer(sizeof(CameraUniforms));
  cameraUniformDescriptorSet->bindUniformBuffer(cameraUniformBuffer, 0);

  auto materialDescriptorSet = shader->allocateDescriptorSet(1);
  auto texture = Illusion::Graphics::Texture::createFromFile(context, "data/textures/box.dds");
  materialDescriptorSet->bindCombinedImageSampler(texture, 0);

  Illusion::Graphics::GraphicsState state;
  state.setShaderProgram(shader);

  Illusion::Graphics::GraphicsState::InputAssemblyState inputAssemblyState;
  inputAssemblyState.mTopology = vk::PrimitiveTopology::eTriangleList;
  state.setInputAssemblyState(inputAssemblyState);

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

  Illusion::Graphics::GraphicsState::VertexInputState vertexInputState;
  vertexInputState.mBindings   = {{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
                                {1, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
                                {2, sizeof(glm::vec2), vk::VertexInputRate::eVertex}};
  vertexInputState.mAttributes = {{0, 0, vk::Format::eR32G32B32Sfloat, 0},
                                  {1, 1, vk::Format::eR32G32B32Sfloat, 0},
                                  {2, 2, vk::Format::eR32G32Sfloat, 0}};
  state.setVertexInputState(vertexInputState);

  auto positionBuffer =
    context->createVertexBuffer(sizeof(glm::vec3) * POSITIONS.size(), POSITIONS.data());
  auto normalBuffer =
    context->createVertexBuffer(sizeof(glm::vec3) * NORMALS.size(), NORMALS.data());
  auto texcoordBuffer =
    context->createVertexBuffer(sizeof(glm::vec2) * TEXCOORDS.size(), TEXCOORDS.data());
  auto indexBuffer = context->createIndexBuffer(sizeof(uint32_t) * INDICES.size(), INDICES.data());

  auto renderPass = window->getDisplayPass();
  renderPass->addAttachment(vk::Format::eD32Sfloat);

  float time = 0.f;

  renderPass->beforeFunc = [&time, &cameraUniformBuffer, &window](
                             std::shared_ptr<Illusion::Graphics::CommandBuffer> cmd,
                             Illusion::Graphics::RenderPass const&              pass) {
    time += 0.01;

    CameraUniforms cameraUniforms;
    cameraUniforms.projection = glm::perspective(
      glm::radians(60.f),
      static_cast<float>(window->pSize.get().x) / static_cast<float>(window->pSize.get().y),
      0.1f,
      100.0f);
    cmd->updateBuffer(
      *cameraUniformBuffer->mBuffer, 0, sizeof(CameraUniforms), (uint8_t*)&cameraUniforms);
  };

  renderPass->drawFunc = [&state,
                          &materialDescriptorSet,
                          &cameraUniformDescriptorSet,
                          &positionBuffer,
                          &normalBuffer,
                          &texcoordBuffer,
                          &indexBuffer,
                          &time](
                           std::shared_ptr<Illusion::Graphics::CommandBuffer> cmd,
                           Illusion::Graphics::RenderPass const&              pass,
                           uint32_t                                           subPass) {

    PushConstants pushConstants;
    pushConstants.modelView = glm::mat4(1.f);
    pushConstants.modelView = glm::translate(pushConstants.modelView, glm::vec3(0, 0, -3));
    pushConstants.modelView =
      glm::rotate(pushConstants.modelView, -time * 0.5f, glm::vec3(0, 1, 0));
    pushConstants.modelView =
      glm::rotate(pushConstants.modelView, time * 0.314f, glm::vec3(1, 0, 0));

    auto pipeline = pass.getPipelineHandle(state, subPass);
    cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmd->pushConstants(
      *state.getShaderProgram()->getPipelineLayout(),
      vk::ShaderStageFlagBits::eVertex,
      pushConstants);
    cmd->bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      *state.getShaderProgram()->getPipelineLayout(),
      materialDescriptorSet->getSet(),
      *materialDescriptorSet,
      nullptr);
    cmd->bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      *state.getShaderProgram()->getPipelineLayout(),
      cameraUniformDescriptorSet->getSet(),
      *cameraUniformDescriptorSet,
      nullptr);
    cmd->bindVertexBuffers(
      0,
      {*positionBuffer->mBuffer, *normalBuffer->mBuffer, *texcoordBuffer->mBuffer},
      {0uL, 0uL, 0uL});
    cmd->bindIndexBuffer(*indexBuffer->mBuffer, 0, vk::IndexType::eUint32);
    cmd->drawIndexed(INDICES.size(), 1, 0, 0, 0);
  };

  while (!window->shouldClose()) {
    window->processInput();
    renderPass->render();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  context->getDevice()->waitIdle();

  return 0;
}
