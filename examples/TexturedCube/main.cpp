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
#include <Illusion/Core/RingBuffer.hpp>
#include <Illusion/Graphics/CoherentUniformBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/PipelineReflection.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
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

struct FrameResources {
  FrameResources(std::shared_ptr<Illusion::Graphics::Device> const& device)
    : mRenderPass(std::make_shared<Illusion::Graphics::RenderPass>(device))
    , mUniformBuffer(
        std::make_shared<Illusion::Graphics::CoherentUniformBuffer>(device, sizeof(CameraUniforms)))
    , mCmd(std::make_shared<Illusion::Graphics::CommandBuffer>(device))
    , mRenderFinishedFence(device->createFence({vk::FenceCreateFlagBits::eSignaled}))
    , mRenderFinishedSemaphore(device->createSemaphore({})) {

    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);

    mCmd->graphicsState().addBlendAttachment({});
  }

  std::shared_ptr<Illusion::Graphics::RenderPass>            mRenderPass;
  std::shared_ptr<Illusion::Graphics::CoherentUniformBuffer> mUniformBuffer;
  std::shared_ptr<Illusion::Graphics::CommandBuffer>         mCmd;
  std::shared_ptr<vk::Fence>                                 mRenderFinishedFence;
  std::shared_ptr<vk::Semaphore>                             mRenderFinishedSemaphore;
};

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto engine = std::make_shared<Illusion::Graphics::Engine>("Textured Cube Demo");
  auto device = std::make_shared<Illusion::Graphics::Device>(engine->getPhysicalDevice());
  auto window = std::make_shared<Illusion::Graphics::Window>(engine, device);

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    device,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/TexturedCube.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/TexturedCube.frag"}});
  auto texture = Illusion::Graphics::Texture::createFromFile(device, "data/textures/box.dds");

  auto positionBuffer =
    device->createVertexBuffer(sizeof(glm::vec3) * POSITIONS.size(), POSITIONS.data());
  auto normalBuffer =
    device->createVertexBuffer(sizeof(glm::vec3) * NORMALS.size(), NORMALS.data());
  auto texcoordBuffer =
    device->createVertexBuffer(sizeof(glm::vec2) * TEXCOORDS.size(), TEXCOORDS.data());
  auto indexBuffer = device->createIndexBuffer(sizeof(uint32_t) * INDICES.size(), INDICES.data());

  Illusion::Core::RingBuffer<FrameResources, 2> frameResources{FrameResources(device),
                                                               FrameResources(device)};

  float time = 0.f;

  window->open();
  while (!window->shouldClose()) {
    window->processInput();

    auto& res = frameResources.next();

    device->waitForFences(*res.mRenderFinishedFence, true, ~0);
    device->resetFences(*res.mRenderFinishedFence);

    res.mCmd->reset();
    res.mCmd->begin();

    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setShaderProgram(shader);
    res.mCmd->graphicsState().setViewports(
      {{glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f}});
    res.mCmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleList);
    res.mCmd->graphicsState().setVertexInputBindings(
      {{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
       {1, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
       {2, sizeof(glm::vec2), vk::VertexInputRate::eVertex}});
    res.mCmd->graphicsState().setVertexInputAttributes({{0, 0, vk::Format::eR32G32B32Sfloat, 0},
                                                        {1, 1, vk::Format::eR32G32B32Sfloat, 0},
                                                        {2, 2, vk::Format::eR32G32Sfloat, 0}});

    time += 0.01;

    res.mCmd->bindingState().setUniformBuffer(
      res.mUniformBuffer->getBuffer(), sizeof(CameraUniforms), 0, 0, 0);
    res.mCmd->bindingState().setTexture(texture, 1, 0);

    CameraUniforms cameraUniforms;
    cameraUniforms.projection = glm::perspective(
      glm::radians(60.f),
      static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
      0.1f,
      100.0f);
    res.mUniformBuffer->updateData(cameraUniforms);

    res.mCmd->beginRenderPass(res.mRenderPass);

    PushConstants pushConstants;
    pushConstants.modelView = glm::mat4(1.f);
    pushConstants.modelView = glm::translate(pushConstants.modelView, glm::vec3(0, 0, -3));
    pushConstants.modelView =
      glm::rotate(pushConstants.modelView, -time * 0.5f, glm::vec3(0, 1, 0));
    pushConstants.modelView =
      glm::rotate(pushConstants.modelView, time * 0.314f, glm::vec3(1, 0, 0));

    res.mCmd->pushConstants(vk::ShaderStageFlagBits::eVertex, pushConstants);
    res.mCmd->bindVertexBuffers(
      0,
      {*positionBuffer->mBuffer, *normalBuffer->mBuffer, *texcoordBuffer->mBuffer},
      {0uL, 0uL, 0uL});
    res.mCmd->bindIndexBuffer(*indexBuffer->mBuffer, 0, vk::IndexType::eUint32);
    res.mCmd->drawIndexed(INDICES.size(), 1, 0, 0, 0);
    res.mCmd->endRenderPass();
    res.mCmd->end();

    res.mCmd->submit({}, {}, {*res.mRenderFinishedSemaphore});

    window->present(
      res.mRenderPass->getFramebuffer()->getImages()[0],
      res.mRenderFinishedSemaphore,
      res.mRenderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}
