////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <Illusion/Core/CommandLineOptions.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CoherentUniformBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/FrameResource.hpp>
#include <Illusion/Graphics/GltfModel.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/LazyRenderPass.hpp>
#include <Illusion/Graphics/PhysicalDevice.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <thread>

struct PushConstants {
  glm::mat4 mModelMatrix;
  glm::vec4 mAlbedoFactor;
  glm::vec3 mEmissiveFactor;
  bool      mSpecularGlossinessWorkflow;
  glm::vec3 mMetallicRoughnessFactor;
  float     mNormalScale;
  float     mOcclusionStrength;
  float     mAlphaCutoff;
  int32_t   mVertexAttributes;
};

struct SkinUniforms {
  glm::mat4 mJointMatrices[256];
};

struct CameraUniforms {
  glm::vec4 mPosition;
  glm::mat4 mViewMatrix;
  glm::mat4 mProjectionMatrix;
};

struct PerFrame {
  PerFrame() = default;

  PerFrame(uint32_t index, Illusion::Graphics::DevicePtr const& device, vk::DeviceSize uboAlignment)
      : mCmd(Illusion::Graphics::CommandBuffer::create(
            "CommandBuffer " + std::to_string(index), device))
      , mRenderPass(Illusion::Graphics::LazyRenderPass::create(
            "RenderPass " + std::to_string(index), device))
      , mUniformBuffer(Illusion::Graphics::CoherentUniformBuffer::create(
            "CoherentUniformBuffer " + std::to_string(index), device, std::pow(2, 20),
            uboAlignment))
      , mRenderFinishedFence(device->createFence("RenderFinished " + std::to_string(index)))
      , mRenderFinishedSemaphore(
            device->createSemaphore("FrameFinished " + std::to_string(index))) {

    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);
  }

  Illusion::Graphics::CommandBufferPtr         mCmd;
  Illusion::Graphics::LazyRenderPassPtr        mRenderPass;
  Illusion::Graphics::CoherentUniformBufferPtr mUniformBuffer;
  vk::FencePtr                                 mRenderFinishedFence;
  vk::SemaphorePtr                             mRenderFinishedSemaphore;
};

void drawNodes(std::vector<std::shared_ptr<Illusion::Graphics::Gltf::Node>> const& nodes,
    glm::mat4 const& viewMatrix, glm::mat4 const& modelMatrix, bool doAlphaBlending,
    vk::DeviceSize emptySkinDynamicOffset, PerFrame const& res) {

  res.mCmd->graphicsState().setBlendAttachments({{doAlphaBlending}});

  for (auto const& n : nodes) {

    glm::mat4 nodeMatrix = n->mGlobalTransform;

    if (n->mSkin) {
      SkinUniforms skin{};
      auto         jointMatrices = n->mSkin->getJointMatrices(nodeMatrix);

      for (size_t i(0); i < jointMatrices.size() && i < 256; ++i) {
        skin.mJointMatrices[i] = jointMatrices[i];
      }

      auto skinDynamicOffset = res.mUniformBuffer->addData(skin);
      res.mCmd->bindingState().setDynamicUniformBuffer(res.mUniformBuffer->getBuffer(),
          sizeof(SkinUniforms), static_cast<uint32_t>(skinDynamicOffset), 2, 0);
    } else {
      res.mCmd->bindingState().setDynamicUniformBuffer(res.mUniformBuffer->getBuffer(),
          sizeof(SkinUniforms), static_cast<uint32_t>(emptySkinDynamicOffset), 2, 0);
    }

    if (n->mMesh) {

      for (auto const& p : n->mMesh->mPrimitives) {

        if (p.mMaterial->mDoAlphaBlending == doAlphaBlending) {
          PushConstants pushConstants{};
          pushConstants.mModelMatrix                = modelMatrix * nodeMatrix;
          pushConstants.mAlbedoFactor               = p.mMaterial->mAlbedoFactor;
          pushConstants.mEmissiveFactor             = p.mMaterial->mEmissiveFactor;
          pushConstants.mSpecularGlossinessWorkflow = p.mMaterial->mSpecularGlossinessWorkflow;
          pushConstants.mMetallicRoughnessFactor    = p.mMaterial->mMetallicRoughnessFactor;
          pushConstants.mNormalScale                = p.mMaterial->mNormalScale;
          pushConstants.mOcclusionStrength          = p.mMaterial->mOcclusionStrength;
          pushConstants.mAlphaCutoff                = p.mMaterial->mAlphaCutoff;
          pushConstants.mVertexAttributes           = static_cast<int32_t>(p.mVertexAttributes);
          res.mCmd->pushConstants(pushConstants);

          res.mCmd->bindingState().setTexture(p.mMaterial->mAlbedoTexture, 3, 0);
          res.mCmd->bindingState().setTexture(p.mMaterial->mMetallicRoughnessTexture, 3, 1);
          res.mCmd->bindingState().setTexture(p.mMaterial->mNormalTexture, 3, 2);
          res.mCmd->bindingState().setTexture(p.mMaterial->mOcclusionTexture, 3, 3);
          res.mCmd->bindingState().setTexture(p.mMaterial->mEmissiveTexture, 3, 4);
          res.mCmd->graphicsState().setTopology(p.mTopology);
          res.mCmd->graphicsState().setCullMode(p.mMaterial->mDoubleSided
                                                    ? vk::CullModeFlagBits::eNone
                                                    : vk::CullModeFlagBits::eBack);
          res.mCmd->drawIndexed(p.mIndexCount, 1, p.mIndexOffset, 0, 0);
        }
      }
    }

    drawNodes(n->mChildren, viewMatrix, modelMatrix, doAlphaBlending, emptySkinDynamicOffset, res);
  }
};

int main(int argc, char* argv[]) {

  struct {
    std::string mModelFile   = "data/GltfViewer/models/DamagedHelmet.glb";
    std::string mSkyboxFile  = "data/GltfViewer/textures/sunset_fairway_1k.hdr";
    std::string mTexChannels = "rgb";
    int32_t     mAnimation   = 0;
    bool        mNoSkins     = false;
    bool        mNoTextures  = false;
    bool        mPrintInfo   = false;
    bool        mPrintHelp   = false;
  } options;

  // clang-format off
  Illusion::Core::CommandLineOptions args("Simple viewer for GLTF files.");
  args.addOption({"-h",  "--help"},        &options.mPrintHelp,  "Print this help");
  args.addOption({"-i",  "--info"},        &options.mPrintInfo,  "Print information on the loaded model");
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

  auto model =
      Illusion::Graphics::Gltf::Model::create("GltfModel", device, options.mModelFile, loadOptions);

  if (options.mPrintInfo) {
    model->printInfo();
  }

  auto      modelBBox   = model->getRoot()->getBoundingBox();
  float     modelSize   = glm::length(modelBBox.mMin - modelBBox.mMax);
  glm::vec3 modelCenter = (modelBBox.mMin + modelBBox.mMax) * 0.5f;
  glm::mat4 modelMatrix = glm::scale(glm::vec3(1.f / modelSize));
  modelMatrix           = glm::translate(modelMatrix, -modelCenter);

  auto brdflut = Illusion::Graphics::Texture::createBRDFLuT("BRDFLuT", device, 128);
  auto skybox  = Illusion::Graphics::Texture::createCubemapFrom360PanoramaFile(
      "SkyboxTexture", device, options.mSkyboxFile, 1024);
  auto prefilteredIrradiance = Illusion::Graphics::Texture::createPrefilteredIrradianceCubemap(
      "IrradianceTexture", device, 64, skybox);
  auto prefilteredReflection = Illusion::Graphics::Texture::createPrefilteredReflectionCubemap(
      "ReflectionTexture", device, 128, skybox);

  auto pbrShader = Illusion::Graphics::Shader::createFromFiles("PBRShader", device,
      {"data/GltfViewer/shaders/GltfShader.vert", "data/GltfViewer/shaders/GltfShader.frag"},
      {"SkinUniforms"});

  auto skyShader = Illusion::Graphics::Shader::createFromFiles("SkyboxShader", device,
      {"data/GltfViewer/shaders/Skybox.vert", "data/GltfViewer/shaders/Skybox.frag"});

  auto uboAlignment =
      instance->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

  auto frameIndex = Illusion::Graphics::FrameResourceIndex::create(2);
  Illusion::Graphics::FrameResource<PerFrame> perFrame(
      frameIndex, [=](uint32_t index) { return PerFrame(index, device, uboAlignment); });

  glm::vec3 cameraPolar(0.f, 0.f, 1.5f);

  window->sOnKeyEvent.connect([&window](Illusion::Input::KeyEvent const& e) {
    if (e.mType == Illusion::Input::KeyEvent::Type::ePress &&
        e.mKey == Illusion::Input::Key::eF11) {
      window->pFullscreen = !window->pFullscreen.get();
    }
    return true;
  });

  window->sOnMouseEvent.connect([&cameraPolar, &window](Illusion::Input::MouseEvent const& e) {
    if (e.mType == Illusion::Input::MouseEvent::Type::eMove) {
      static int32_t lastX = e.mX;
      static int32_t lastY = e.mY;

      if (window->buttonPressed(Illusion::Input::Button::eButton1)) {
        int32_t dX = lastX - e.mX;
        int32_t dY = lastY - e.mY;

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

  Illusion::Core::Timer timer;

  while (!window->shouldClose()) {

    window->update();

    frameIndex->step();

    if (options.mAnimation >= 0 &&
        static_cast<size_t>(options.mAnimation) < model->getAnimations().size()) {
      auto const& anim = model->getAnimations()[options.mAnimation];
      float       modelAnimationTime =
          std::fmod(static_cast<float>(timer.getElapsed()), anim->mEnd - anim->mStart);
      modelAnimationTime += anim->mStart;
      model->setAnimationTime(options.mAnimation, modelAnimationTime);
    }

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

    camera.mPosition =
        glm::vec4(glm::vec3(std::cos(cameraPolar.y) * std::sin(cameraPolar.x),
                      -std::sin(cameraPolar.y), std::cos(cameraPolar.y) * std::cos(cameraPolar.x)) *
                      cameraPolar.z,
            1.0);

    res.mUniformBuffer->reset();

    camera.mViewMatrix =
        glm::lookAt(camera.mPosition.xyz(), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    res.mUniformBuffer->addData(camera);

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

    res.mCmd->draw(4);

    res.mCmd->bindingState().reset(1);

    res.mCmd->setShader(pbrShader);
    res.mCmd->bindingState().setTexture(brdflut, 1, 0);
    res.mCmd->bindingState().setTexture(prefilteredIrradiance, 1, 1);
    res.mCmd->bindingState().setTexture(prefilteredReflection, 1, 2);
    res.mCmd->graphicsState().setDepthTestEnable(true);
    res.mCmd->graphicsState().setDepthWriteEnable(true);
    res.mCmd->graphicsState().setVertexInputAttributes(
        Illusion::Graphics::Gltf::Model::getVertexInputAttributes());
    res.mCmd->graphicsState().setVertexInputBindings(
        Illusion::Graphics::Gltf::Model::getVertexInputBindings());

    res.mCmd->bindVertexBuffers(0, {model->getVertexBuffer()});
    res.mCmd->bindIndexBuffer(model->getIndexBuffer(), 0, vk::IndexType::eUint32);

    SkinUniforms ubo{};
    uint32_t     emptySkinDynamicOffset = res.mUniformBuffer->addData(ubo);

    drawNodes(model->getRoot()->mChildren, camera.mViewMatrix, modelMatrix, false,
        emptySkinDynamicOffset, res);
    drawNodes(model->getRoot()->mChildren, camera.mViewMatrix, modelMatrix, true,
        emptySkinDynamicOffset, res);

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
