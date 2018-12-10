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

  std::string modelFile = "data/models/DamagedHelmet.glb";
  bool        printHelp = false;

  Illusion::Core::CommandLineOptions args("Simple loader for GLTF files.");
  args.addOption({"-m", "--model"}, &modelFile, "The model to load");
  args.addOption({"-h", "--help"}, &printHelp, "print help");

  args.parse(argc, argv);

  if (printHelp) {
    args.printHelp();
    return 0;
  }

  auto engine = Illusion::Graphics::Engine::create("Simple GLTF Loader");
  auto device = Illusion::Graphics::Device::create(engine->getPhysicalDevice());
  auto window = Illusion::Graphics::Window::create(engine, device);

  auto model  = Illusion::Graphics::GltfModel::create(device, modelFile);
  auto shader = Illusion::Graphics::ShaderProgram::createFromFiles(
    device, {"data/shaders/SimpleGltfShader.vert", "data/shaders/SimpleGltfShader.frag"});

  Illusion::Core::RingBuffer<FrameResources, 2> frameResources{
    FrameResources(device), FrameResources(device)};

  float time = 0.f;

  glm::vec3 cameraPolar(0.f, 0.f, 3.f);

  window->sOnMouseEvent.connect([&](Illusion::Input::MouseEvent const& e) {
    if (e.mType == Illusion::Input::MouseEvent::Type::eMove) {
      static int lastX = e.mX;
      static int lastY = e.mY;

      if (window->buttonPressed(Illusion::Input::Button::eButton1)) {
        int dX = lastX - e.mX;
        int dY = lastY - e.mY;

        cameraPolar.x += dX * 0.005f;
        cameraPolar.y += dY * 0.005f;

        cameraPolar.y = glm::clamp(
          cameraPolar.y, -glm::pi<float>() * 0.5f + 0.1f, glm::pi<float>() * 0.5f - 0.1f);
      }

      lastX = e.mX;
      lastY = e.mY;
    } else if (e.mType == Illusion::Input::MouseEvent::Type::eScroll) {
      cameraPolar.z -= e.mY * 0.01;
      cameraPolar.z = std::max(cameraPolar.z, 0.01f);
    }

    return true;
  });

  window->open();

  while (!window->shouldClose()) {

    window->processInput();

    time += 0.01;
    auto& res = frameResources.next();

    device->waitForFences(*res.mRenderFinishedFence, true, ~0);
    device->resetFences(*res.mRenderFinishedFence);

    res.mCmd->reset();
    res.mCmd->begin();

    res.mCmd->setShaderProgram(shader);
    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setViewports(
      {{glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f}});

    glm::mat4 projection = glm::perspectiveZO(glm::radians(50.f),
      static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
      0.1f, 100.0f);
    projection[1][1] *= -1;
    res.mUniformBuffer->updateData(projection);

    glm::vec3 cameraCartesian =
      glm::vec3(-std::cos(cameraPolar.y) * std::sin(cameraPolar.x), -std::sin(cameraPolar.y),
        -std::cos(cameraPolar.y) * std::cos(cameraPolar.x)) *
      cameraPolar.z;

    glm::mat4 view = glm::lookAt(cameraCartesian, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

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
