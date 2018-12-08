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
#include <Illusion/Graphics/GltfModel.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <thread>

struct FrameResources {
  FrameResources(Illusion::Graphics::DevicePtr const& device)
    : mCmd(Illusion::Graphics::CommandBuffer::create(device))
    , mRenderPass(Illusion::Graphics::RenderPass::create(device))
    , mUniformBuffer(Illusion::Graphics::CoherentUniformBuffer::create(device, 512))
    , mRenderFinishedFence(device->createFence())
    , mRenderFinishedSemaphore(device->createSemaphore()) {

    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);

    mCmd->graphicsState().addBlendAttachment({});
    mCmd->graphicsState().setVertexInputBindings(
      Illusion::Graphics::GltfModel::getVertexInputBindings());
    mCmd->graphicsState().setVertexInputAttributes(
      Illusion::Graphics::GltfModel::getVertexInputAttributes());
  }

  Illusion::Graphics::CommandBufferPtr         mCmd;
  Illusion::Graphics::RenderPassPtr            mRenderPass;
  Illusion::Graphics::CoherentUniformBufferPtr mUniformBuffer;
  vk::FencePtr                                 mRenderFinishedFence;
  vk::SemaphorePtr                             mRenderFinishedSemaphore;
};

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto engine = Illusion::Graphics::Engine::create("Triangle Demo");
  auto device = Illusion::Graphics::Device::create(engine->getPhysicalDevice());
  auto window = Illusion::Graphics::Window::create(engine, device);

  auto model  = Illusion::Graphics::GltfModel::create(device, "data/models/DamagedHelmet.glb");
  auto shader = Illusion::Graphics::ShaderProgram::createFromFiles(
    device, {"data/shaders/SimpleGltfShader.vert", "data/shaders/SimpleGltfShader.frag"});

  Illusion::Core::RingBuffer<FrameResources, 2> frameResources{
    FrameResources(device), FrameResources(device)};

  float time = 0.f;

  window->open();

  while (!window->shouldClose()) {

    window->processInput();

    time += 0.01;
    auto& res = frameResources.next();

    device->waitForFences(*res.mRenderFinishedFence, true, ~0);
    device->resetFences(*res.mRenderFinishedFence);

    res.mCmd->reset();
    res.mCmd->begin();

    res.mCmd->graphicsState().setShaderProgram(shader);
    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setViewports(
      {{glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f}});

    glm::mat4 projection = glm::perspective(glm::radians(60.f),
      static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
      0.1f, 100.0f);
    res.mUniformBuffer->updateData(projection);

    glm::mat4 view(1.f);
    view = glm::translate(view, glm::vec3(0, 0, -3));
    view = glm::rotate(view, -time * 0.5f, glm::vec3(0, 1, 0));
    view = glm::rotate(view, time * 0.314f, glm::vec3(1, 0, 0));

    res.mCmd->bindingState().setUniformBuffer(
      res.mUniformBuffer->getBuffer(), sizeof(glm::mat4), 0, 0, 0);

    res.mCmd->beginRenderPass(res.mRenderPass);

    res.mCmd->bindVertexBuffers(0, {model->getVertexBuffer()});
    res.mCmd->bindIndexBuffer(model->getIndexBuffer(), 0, vk::IndexType::eUint32);

    std::function<void(std::vector<Illusion::Graphics::GltfModel::Node> const&)> drawNodes =
      [&](std::vector<Illusion::Graphics::GltfModel::Node> const& nodes) {
        for (auto const& n : nodes) {

          glm::mat4 modelView = view * glm::mat4(n.mModelMatrix);
          res.mCmd->pushConstants(modelView);

          for (auto const& p : n.mPrimitives) {
            res.mCmd->bindingState().setTexture(p.mMaterial->mBaseColorTexture, 1, 0);
            res.mCmd->bindingState().setTexture(p.mMaterial->mOcclusionTexture, 1, 1);
            res.mCmd->bindingState().setTexture(p.mMaterial->mEmissiveTexture, 1, 2);
            res.mCmd->graphicsState().setTopology(p.mTopology);
            res.mCmd->drawIndexed(p.mIndexCount, 1, p.mIndexOffset, 0, 0);
          }

          drawNodes(n.mChildren);
        }
      };

    drawNodes(model->getNodes());

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
