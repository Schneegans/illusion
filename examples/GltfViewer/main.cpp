////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GltfModel.hpp"
#include "Turntable.hpp"

#include <Illusion/Core/CommandLineOptions.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CoherentBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/LazyRenderPass.hpp>
#include <Illusion/Graphics/PhysicalDevice.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This (rather complex) example loads a glTF-file and displays it using physically based         //
// shading. Both, roughness-metallic and secular-glossiness workflows are supported. It also      //
// loads animations and skins from the file. Most glTFSample Models                               //
// (https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0) are supported, especially //
// the feature-test models render fine.                                                           //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CameraUniforms {
  glm::vec4 mPosition;
  glm::mat4 mViewMatrix;
  glm::mat4 mProjectionMatrix;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct PerFrame {
  PerFrame() = default;

  PerFrame(uint32_t index, Illusion::Graphics::DevicePtr const& device)
      : mCmd(Illusion::Graphics::CommandBuffer::create(
            "CommandBuffer " + std::to_string(index), device))
      , mRenderPass(Illusion::Graphics::LazyRenderPass::create(
            "RenderPass " + std::to_string(index), device))
      , mUniformBuffer(Illusion::Graphics::CoherentBuffer::create(
            "CameraUniformBuffer " + std::to_string(index), device, sizeof(CameraUniforms),
            vk::BufferUsageFlagBits::eUniformBuffer))
      , mRenderFinishedFence(device->createFence("RenderFinished " + std::to_string(index)))
      , mRenderFinishedSemaphore(
            device->createSemaphore("FrameFinished " + std::to_string(index))) {

    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);
  }

  Illusion::Graphics::CommandBufferPtr  mCmd;
  Illusion::Graphics::LazyRenderPassPtr mRenderPass;
  Illusion::Graphics::CoherentBufferPtr mUniformBuffer;
  vk::FencePtr                          mRenderFinishedFence;
  vk::SemaphorePtr                      mRenderFinishedSemaphore;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  struct {
    std::string mModelFile  = "data/GltfViewer/models/DamagedHelmet.glb";
    std::string mSkyboxFile = "data/GltfViewer/textures/sunset_fairway_1k.hdr";
    int32_t     mAnimation  = 0;
    bool        mNoSkins    = false;
    bool        mNoTextures = false;
    bool        mPrintHelp  = false;
  } options;

  // clang-format off
  Illusion::Core::CommandLineOptions args("Simple viewer for GLTF files.");
  args.addOption({"-h",  "--help"},        &options.mPrintHelp,  "Print this help");
  args.addOption({"-m",  "--model"},       &options.mModelFile,  "GLTF model (.gltf or .glb)");
  args.addOption({"-e",  "--environment"}, &options.mSkyboxFile, "Skybox image (in equirectangular projection)");
  args.addOption({"-a",  "--animation"},   &options.mAnimation,  "Index of the animation to play. Default: 0, Use -1 to disable animations.");
  args.addOption({"-ns", "--no-skins"},    &options.mNoSkins,    "Disable loading of skins");
  args.addOption({"-nt", "--no-textures"}, &options.mNoTextures, "Disable loading of textures");
  args.addOption({"-t",  "--trace"},       &Illusion::Core::Logger::enableTrace, "Print trace output");
  // clang-format on

  args.parse(argc, argv);

  if (options.mPrintHelp) {
    args.printHelp();
    return 0;
  }

  auto instance = Illusion::Graphics::Instance::create("Simple GLTF Loader");
  auto device   = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create("Window", instance, device);

  Illusion::Graphics::Gltf::LoadOptions loadOptions;
  if (options.mAnimation >= 0) {
    loadOptions |= Illusion::Graphics::Gltf::LoadOptionBits::eAnimations;
  }
  if (!options.mNoSkins) {
    loadOptions |= Illusion::Graphics::Gltf::LoadOptionBits::eSkins;
  }
  if (!options.mNoTextures) {
    loadOptions |= Illusion::Graphics::Gltf::LoadOptionBits::eTextures;
  }

  auto frameIndex = Illusion::Graphics::FrameResourceIndex::create(2);

  auto model = GltfModel("GltfModel", device, options.mModelFile, loadOptions, frameIndex);

  auto brdflut = Illusion::Graphics::Texture::createBRDFLuT("BRDFLuT", device, 128);
  auto skybox  = Illusion::Graphics::Texture::createCubemapFrom360PanoramaFile(
      "SkyboxTexture", device, options.mSkyboxFile, 1024);
  auto prefilteredIrradiance = Illusion::Graphics::Texture::createPrefilteredIrradianceCubemap(
      "IrradianceTexture", device, 64, skybox);
  auto prefilteredReflection = Illusion::Graphics::Texture::createPrefilteredReflectionCubemap(
      "ReflectionTexture", device, 128, skybox);

  auto skyShader = Illusion::Graphics::Shader::createFromFiles("SkyboxShader", device,
      {"data/GltfViewer/shaders/Skybox.vert", "data/GltfViewer/shaders/Skybox.frag"});

  Illusion::Graphics::FrameResource<PerFrame> perFrame(
      frameIndex, [=](uint32_t index) { return PerFrame(index, device); });

  window->sOnKeyEvent.connect([&window](Illusion::Input::KeyEvent const& e) {
    if (e.mType == Illusion::Input::KeyEvent::Type::ePress &&
        e.mKey == Illusion::Input::Key::eF11) {
      window->pFullscreen = !window->pFullscreen.get();
    }
    return true;
  });

  Turntable turntable(window);

  window->open();

  Illusion::Core::Timer timer;

  while (!window->shouldClose()) {

    window->update();

    frameIndex->step();

    model.update(timer.getElapsed(), options.mAnimation);

    auto& res = perFrame.current();

    device->waitForFence(res.mRenderFinishedFence);
    device->resetFence(res.mRenderFinishedFence);

    res.mCmd->reset();
    res.mCmd->begin();

    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setViewports({{glm::vec2(window->pExtent.get())}});

    CameraUniforms camera{};
    camera.mProjectionMatrix = glm::perspectiveZO(glm::radians(50.f),
        static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
        0.01f, 10.0f);
    camera.mProjectionMatrix[1][1] *= -1;

    camera.mPosition   = turntable.getCameraPosition();
    camera.mViewMatrix = turntable.getViewMatrix();

    res.mUniformBuffer->updateData(camera);

    // The color and depth our framebuffer attachments will be cleared to.
    std::vector<vk::ClearValue> clearValues;
    clearValues.emplace_back(vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}}));
    clearValues.emplace_back(vk::ClearDepthStencilValue(1.f, 0u));

    res.mCmd->beginRenderPass(res.mRenderPass, clearValues);

    res.mCmd->bindingState().setUniformBuffer(
        res.mUniformBuffer->getBuffer(), sizeof(CameraUniforms), 0, 0, 0);

    res.mCmd->setShader(skyShader);
    res.mCmd->bindingState().setTexture(skybox, 1, 0);
    res.mCmd->graphicsState().setDepthTestEnable(false);
    res.mCmd->graphicsState().setDepthWriteEnable(false);
    res.mCmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleStrip);
    res.mCmd->graphicsState().setVertexInputAttributes({});
    res.mCmd->graphicsState().setVertexInputBindings({});
    res.mCmd->draw(3);

    res.mCmd->bindingState().reset(1);
    res.mCmd->bindingState().setTexture(brdflut, 1, 0);
    res.mCmd->bindingState().setTexture(prefilteredIrradiance, 1, 1);
    res.mCmd->bindingState().setTexture(prefilteredReflection, 1, 2);
    res.mCmd->graphicsState().setDepthTestEnable(true);
    res.mCmd->graphicsState().setDepthWriteEnable(true);
    model.draw(res.mCmd, camera.mViewMatrix);

    res.mCmd->endRenderPass();
    res.mCmd->end();

    res.mCmd->submit({}, {}, {res.mRenderFinishedSemaphore});

    window->present(res.mRenderPass->getAttachments()[0].mImage, res.mRenderFinishedSemaphore,
        res.mRenderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}
