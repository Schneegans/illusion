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
#include <Illusion/Graphics/DescriptorSet.hpp>
#include <Illusion/Graphics/DescriptorSetCache.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GltfModel.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/PipelineReflection.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <thread>

struct FrameResources {
  FrameResources(std::shared_ptr<Illusion::Graphics::Device> const& device)
    : mRenderPass(std::make_shared<Illusion::Graphics::RenderPass>(device))
    , mDescriptorSetCache(std::make_shared<Illusion::Graphics::DescriptorSetCache>(device))
    , mUniformBuffer(std::make_shared<Illusion::Graphics::CoherentUniformBuffer>(device, 512))
    , mCommandBuffer(device->allocateGraphicsCommandBuffer())
    , mRenderFinishedFence(device->createFence({vk::FenceCreateFlagBits::eSignaled}))
    , mRenderFinishedSemaphore(device->createSemaphore({})) {

    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);
  }

  std::shared_ptr<Illusion::Graphics::RenderPass>            mRenderPass;
  std::shared_ptr<Illusion::Graphics::DescriptorSetCache>    mDescriptorSetCache;
  std::shared_ptr<Illusion::Graphics::CoherentUniformBuffer> mUniformBuffer;
  std::shared_ptr<Illusion::Graphics::CommandBuffer>         mCommandBuffer;
  std::shared_ptr<vk::Fence>                                 mRenderFinishedFence;
  std::shared_ptr<vk::Semaphore>                             mRenderFinishedSemaphore;
  int32_t                                                    mSceneUniformOffset = -1;
};

struct CameraUniforms {
  glm::mat4 mProjection;
};

struct PushConstants {
  glm::mat4 mModelView;
};

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto engine = std::make_shared<Illusion::Graphics::Engine>("Triangle Demo");
  auto device = std::make_shared<Illusion::Graphics::Device>(engine->getPhysicalDevice());
  auto window = std::make_shared<Illusion::Graphics::Window>(engine, device);

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    device,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/SimpleGltfShader.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/SimpleGltfShader.frag"}});

  auto model =
    std::make_shared<Illusion::Graphics::GltfModel>(device, "data/models/DamagedHelmet.glb");

  Illusion::Graphics::GraphicsState state;
  state.setShaderProgram(shader);
  state.addBlendAttachment({});
  state.addViewport({glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f});
  state.setVertexInputBindings(Illusion::Graphics::GltfModel::getVertexInputBindings());
  state.setVertexInputAttributes(Illusion::Graphics::GltfModel::getVertexInputAttributes());
  state.setTopology(vk::PrimitiveTopology::eTriangleList);

  window->pExtent.onChange().connect([&state](glm::uvec2 const& size) {
    auto viewports       = state.getViewports();
    viewports[0].mExtend = size;
    state.setViewports(viewports);
    return true;
  });

  Illusion::Core::RingBuffer<FrameResources, 2> frameResources{FrameResources(device),
                                                               FrameResources(device)};

  float time = 0.f;

  window->open();
  while (!window->shouldClose()) {
    window->processInput();

    time += 0.01;
    auto& res = frameResources.next();

    device->waitForFences(*res.mRenderFinishedFence, true, ~0);
    device->resetFences(*res.mRenderFinishedFence);

    res.mRenderPass->setExtent(window->pExtent.get());

    res.mCommandBuffer->reset({});
    res.mCommandBuffer->begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    // auto materialUniformDescriptorSet =
    //   res.mDescriptorSetCache->acquireHandle(shader->getDescriptorSetReflections().at(1));
    // auto materialUniformBuffer = res.mBufferCache->acquireHandle(
    //   sizeof(Illusion::Graphics::GltfModel::Material::UniformBuffer),
    //   vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
    //   vk::MemoryPropertyFlagBits::eDeviceLocal);
    // materialUniformDescriptorSet->bindUniformBuffer(materialUniformBuffer, 0);

    CameraUniforms cameraUniforms;
    cameraUniforms.mProjection = glm::perspective(
      glm::radians(60.f),
      static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
      0.1f,
      100.0f);

    glm::mat4 view(1.f);
    view = glm::translate(view, glm::vec3(0, 0, -3));
    view = glm::rotate(view, -time * 0.5f, glm::vec3(0, 1, 0));
    view = glm::rotate(view, time * 0.314f, glm::vec3(1, 0, 0));

    if (res.mSceneUniformOffset < 0) {
      res.mSceneUniformOffset = res.mUniformBuffer->addData(cameraUniforms);
    } else {
      res.mUniformBuffer->updateData(cameraUniforms, res.mSceneUniformOffset);
    }

    auto sceneUniformDescriptorSet =
      res.mDescriptorSetCache->acquireHandle(shader->getDescriptorSetReflections().at(0));
    sceneUniformDescriptorSet->bindUniformBuffer(
      res.mUniformBuffer->getBuffer(), 0, sizeof(CameraUniforms), res.mSceneUniformOffset);

    res.mRenderPass->begin(res.mCommandBuffer);

    auto pipeline = res.mRenderPass->getPipelineHandle(state);
    res.mCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    res.mCommandBuffer->bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      *state.getShaderProgram()->getReflection()->getLayout(),
      sceneUniformDescriptorSet->getSet(),
      *sceneUniformDescriptorSet,
      nullptr);

    model->bindIndexBuffer(res.mCommandBuffer);
    model->bindVertexBuffer(res.mCommandBuffer);

    std::function<void(std::vector<Illusion::Graphics::GltfModel::Node> const&)> drawNodes =
      [&](std::vector<Illusion::Graphics::GltfModel::Node> const& nodes) {
        for (auto const& n : nodes) {

          PushConstants pushConstants;
          pushConstants.mModelView = view * glm::mat4(n.mModelMatrix);

          res.mCommandBuffer->pushConstants(
            *state.getShaderProgram()->getReflection()->getLayout(),
            vk::ShaderStageFlagBits::eVertex,
            pushConstants);

          for (auto const& p : n.mPrimitives) {
            if (p.mTopology == state.getTopology()) {
              res.mCommandBuffer->drawIndexed(p.mIndexCount, 1, p.mIndexOffset, 0, 0);
            } else {
              ILLUSION_WARNING << "Failed to draw primitive: Wrong topology!" << std::endl;
            }
          }

          drawNodes(n.mChildren);
        }
      };

    drawNodes(model->getNodes());

    res.mRenderPass->end(res.mCommandBuffer);
    res.mCommandBuffer->end();

    device->submit({*res.mCommandBuffer}, {}, {}, {*res.mRenderFinishedSemaphore});

    window->present(
      res.mRenderPass->getFramebuffer()->getImages()[0],
      res.mRenderFinishedSemaphore,
      res.mRenderFinishedFence);

    res.mDescriptorSetCache->releaseAll();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}
