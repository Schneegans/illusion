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
#include <Illusion/Graphics/TextureUtils.hpp>
#include <Illusion/Graphics/Window.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <thread>

struct PushConstants {
  glm::mat4                                              mModelView;
  Illusion::Graphics::GltfModel::Material::PushConstants mMaterial;
};

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

void drawNodes(std::vector<std::shared_ptr<Illusion::Graphics::GltfModel::Node>> const& nodes,
  glm::mat4 const& parentMatrix, glm::mat4 const& viewMatrix, FrameResources const& res) {

  for (auto const& n : nodes) {
    auto modelMatrix = parentMatrix * glm::mat4(n->mModelMatrix);

    if (n->mMesh) {

      for (auto const& p : n->mMesh->mPrimitives) {
        PushConstants pushConstants;
        pushConstants.mModelView = viewMatrix * modelMatrix;
        pushConstants.mMaterial  = p.mMaterial->mPushConstants;
        res.mCmd->pushConstants(pushConstants);

        res.mCmd->bindingState().setTexture(p.mMaterial->mAlbedoTexture, 2, 0);
        res.mCmd->bindingState().setTexture(p.mMaterial->mMetallicRoughnessTexture, 2, 1);
        res.mCmd->bindingState().setTexture(p.mMaterial->mNormalTexture, 2, 2);
        res.mCmd->bindingState().setTexture(p.mMaterial->mOcclusionTexture, 2, 3);
        res.mCmd->bindingState().setTexture(p.mMaterial->mEmissiveTexture, 2, 4);
        res.mCmd->graphicsState().setTopology(p.mTopology);
        res.mCmd->drawIndexed(p.mIndexCount, 1, p.mIndexOffset, 0, 0);
      }
    }

    drawNodes(n->mChildren, modelMatrix, viewMatrix, res);
  }
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

  auto      model       = Illusion::Graphics::GltfModel::create(device, modelFile);
  float     modelSize   = glm::length(model->getBoundingBox().mMin - model->getBoundingBox().mMax);
  glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.f / modelSize));

  auto brdflut = Illusion::Graphics::TextureUtils::createBRDFLuT(device, 256);
  auto cubemap = Illusion::Graphics::TextureUtils::createCubemapFrom360PanoramaFile(
    device, "data/textures/whipple_creek_regional_park_04_1k.hdr", 256);
  auto prefilteredIrradiance =
    Illusion::Graphics::TextureUtils::createPrefilteredIrradianceCubemap(device, 64, cubemap);
  auto prefilteredReflection =
    Illusion::Graphics::TextureUtils::createPrefilteredReflectionCubemap(device, cubemap);

  auto shader = Illusion::Graphics::ShaderProgram::createFromFiles(
    device, {"data/shaders/SimpleGltfShader.vert", "data/shaders/SimpleGltfShader.frag"});

  Illusion::Core::RingBuffer<FrameResources, 2> frameResources{
    FrameResources(device), FrameResources(device)};

  glm::vec3 cameraPolar(0.f, 0.f, 1.5f);

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
      glm::vec3(std::cos(cameraPolar.y) * std::sin(cameraPolar.x), -std::sin(cameraPolar.y),
        std::cos(cameraPolar.y) * std::cos(cameraPolar.x)) *
      cameraPolar.z;

    glm::mat4 viewMatrix = glm::lookAt(cameraCartesian, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

    res.mCmd->bindingState().setUniformBuffer(
      res.mUniformBuffer->getBuffer(), sizeof(glm::mat4), 0, 0, 0);

    res.mCmd->bindingState().setTexture(brdflut, 1, 0);
    res.mCmd->bindingState().setTexture(prefilteredIrradiance, 1, 1);
    res.mCmd->bindingState().setTexture(prefilteredReflection, 1, 2);

    res.mCmd->beginRenderPass(res.mRenderPass);

    res.mCmd->bindVertexBuffers(0, {model->getVertexBuffer()});
    res.mCmd->bindIndexBuffer(model->getIndexBuffer(), 0, vk::IndexType::eUint32);

    drawNodes(model->getNodes(), modelMatrix, viewMatrix, res);

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