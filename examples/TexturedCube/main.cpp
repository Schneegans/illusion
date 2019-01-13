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
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
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

struct FrameResources {
  FrameResources(Illusion::Graphics::DevicePtr const& device)
      : mCmd(Illusion::Graphics::CommandBuffer::create(device))
      , mRenderPass(Illusion::Graphics::RenderPass::create(device))
      , mUniformBuffer(Illusion::Graphics::CoherentUniformBuffer::create(device, sizeof(glm::mat4)))
      , mRenderFinishedFence(device->createFence())
      , mRenderFinishedSemaphore(device->createSemaphore()) {

    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);

    mCmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleList);
    mCmd->graphicsState().setVertexInputBindings(
        {{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
            {1, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
            {2, sizeof(glm::vec2), vk::VertexInputRate::eVertex}});
    mCmd->graphicsState().setVertexInputAttributes({{0, 0, vk::Format::eR32G32B32Sfloat, 0},
        {1, 1, vk::Format::eR32G32B32Sfloat, 0}, {2, 2, vk::Format::eR32G32Sfloat, 0}});
  }

  Illusion::Graphics::CommandBufferPtr         mCmd;
  Illusion::Graphics::RenderPassPtr            mRenderPass;
  Illusion::Graphics::CoherentUniformBufferPtr mUniformBuffer;
  vk::FencePtr                                 mRenderFinishedFence;
  vk::SemaphorePtr                             mRenderFinishedSemaphore;
};

int main() {

  Illusion::Core::Logger::enableTrace = true;

  auto instance = Illusion::Graphics::Instance::create("Textured Cube Demo");
  auto device   = Illusion::Graphics::Device::create(instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create(instance, device);

  auto texture = Illusion::Graphics::Texture::createFromFile(device, "data/textures/box.dds");
  auto shader  = Illusion::Graphics::Shader::createFromFiles(
      device, {"data/shaders/TexturedCube.vert", "data/shaders/TexturedCube.frag"});

  auto positionBuffer = device->createVertexBuffer(POSITIONS);
  auto normalBuffer   = device->createVertexBuffer(NORMALS);
  auto texcoordBuffer = device->createVertexBuffer(TEXCOORDS);
  auto indexBuffer    = device->createIndexBuffer(INDICES);

  Illusion::Core::RingBuffer<FrameResources, 2> frameResources{
      FrameResources(device), FrameResources(device)};

  float time = 0.f;

  window->open();

  while (!window->shouldClose()) {

    window->update();

    auto& res = frameResources.next();

    device->waitForFences(*res.mRenderFinishedFence);
    device->resetFences(*res.mRenderFinishedFence);

    res.mCmd->reset();
    res.mCmd->begin();

    res.mCmd->setShader(shader);
    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setViewports({{glm::vec2(window->pExtent.get())}});

    res.mCmd->bindingState().setUniformBuffer(
        res.mUniformBuffer->getBuffer(), sizeof(glm::mat4), 0, 0, 0);
    res.mCmd->bindingState().setTexture(texture, 1, 0);

    glm::mat4 projection = glm::perspectiveZO(glm::radians(60.f),
        static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
        0.1f, 100.0f);
    projection[1][1] *= -1;
    res.mUniformBuffer->updateData(projection);

    res.mCmd->beginRenderPass(res.mRenderPass);

    time += 0.01f;
    glm::mat4 modelView(1.f);
    modelView = glm::translate(modelView, glm::vec3(0, 0, -3));
    modelView = glm::rotate(modelView, -time * 0.5f, glm::vec3(0, 1, 0));
    modelView = glm::rotate(modelView, time * 0.3f, glm::vec3(1, 0, 0));

    res.mCmd->pushConstants(modelView);
    res.mCmd->bindVertexBuffers(0, {positionBuffer, normalBuffer, texcoordBuffer});
    res.mCmd->bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
    res.mCmd->drawIndexed(static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);
    res.mCmd->endRenderPass();
    res.mCmd->end();

    res.mCmd->submit({}, {}, {*res.mRenderFinishedSemaphore});

    window->present(res.mRenderPass->getFramebuffer()->getImages()[0], res.mRenderFinishedSemaphore,
        res.mRenderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}
