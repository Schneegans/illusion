////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <Illusion/Core/CommandLineOptions.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/RingBuffer.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CoherentUniformBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GltfModel.hpp>
#include <Illusion/Graphics/PhysicalDevice.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/TextureUtils.hpp>
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
  int       mVertexAttributes;
};

struct SkinUniforms {
  glm::mat4 mJointMatrices[256];
};

struct CameraUniforms {
  glm::vec4 mPosition;
  glm::mat4 mViewMatrix;
  glm::mat4 mProjectionMatrix;
};

struct FrameResources {
  FrameResources(Illusion::Graphics::DevicePtr const& device, vk::DeviceSize uboAlignment)
    : mCmd(Illusion::Graphics::CommandBuffer::create(device))
    , mRenderPass(Illusion::Graphics::RenderPass::create(device))
    , mUniformBuffer(
        Illusion::Graphics::CoherentUniformBuffer::create(device, std::pow(2, 20), uboAlignment))
    , mRenderFinishedFence(device->createFence())
    , mRenderFinishedSemaphore(device->createSemaphore()) {

    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);

    mCmd->graphicsState().addBlendAttachment({});
  }

  Illusion::Graphics::CommandBufferPtr         mCmd;
  Illusion::Graphics::RenderPassPtr            mRenderPass;
  Illusion::Graphics::CoherentUniformBufferPtr mUniformBuffer;
  vk::FencePtr                                 mRenderFinishedFence;
  vk::SemaphorePtr                             mRenderFinishedSemaphore;
};

void drawNodes(std::vector<std::shared_ptr<Illusion::Graphics::GltfModel::Node>> const& nodes,
  glm::mat4 const& viewMatrix, glm::mat4 const& modelMatrix, bool doAlphaBlending,
  uint32_t emptySkinDynamicOffset, FrameResources const& res) {

  res.mCmd->graphicsState().setBlendAttachments({{doAlphaBlending}});

  for (auto const& n : nodes) {

    glm::mat4 nodeMatrix = n->mGlobalTransform;

    if (n->mSkin) {
      SkinUniforms skin;
      auto         jointMatrices = n->mSkin->getJointMatrices(nodeMatrix);

      for (size_t i(0); i < jointMatrices.size() && i < 256; ++i) {
        skin.mJointMatrices[i] = jointMatrices[i];
      }

      auto skinDynamicOffset = res.mUniformBuffer->addData(skin);
      res.mCmd->bindingState().setDynamicUniformBuffer(
        res.mUniformBuffer->getBuffer(), sizeof(SkinUniforms), skinDynamicOffset, 2, 0);
    } else {
      res.mCmd->bindingState().setDynamicUniformBuffer(
        res.mUniformBuffer->getBuffer(), sizeof(SkinUniforms), emptySkinDynamicOffset, 2, 0);
    }

    if (n->mMesh) {

      for (auto const& p : n->mMesh->mPrimitives) {

        if (p.mMaterial->mDoAlphaBlending == doAlphaBlending) {
          PushConstants pushConstants;
          pushConstants.mModelMatrix                = modelMatrix * nodeMatrix;
          pushConstants.mAlbedoFactor               = p.mMaterial->mAlbedoFactor;
          pushConstants.mEmissiveFactor             = p.mMaterial->mEmissiveFactor;
          pushConstants.mSpecularGlossinessWorkflow = p.mMaterial->mSpecularGlossinessWorkflow;
          pushConstants.mMetallicRoughnessFactor    = p.mMaterial->mMetallicRoughnessFactor;
          pushConstants.mNormalScale                = p.mMaterial->mNormalScale;
          pushConstants.mOcclusionStrength          = p.mMaterial->mOcclusionStrength;
          pushConstants.mAlphaCutoff                = p.mMaterial->mAlphaCutoff;
          pushConstants.mVertexAttributes           = (int)p.mVertexAttributes;
          res.mCmd->pushConstants(pushConstants);

          res.mCmd->bindingState().setTexture(p.mMaterial->mAlbedoTexture, 3, 0);
          res.mCmd->bindingState().setTexture(p.mMaterial->mMetallicRoughnessTexture, 3, 1);
          res.mCmd->bindingState().setTexture(p.mMaterial->mNormalTexture, 3, 2);
          res.mCmd->bindingState().setTexture(p.mMaterial->mOcclusionTexture, 3, 3);
          res.mCmd->bindingState().setTexture(p.mMaterial->mEmissiveTexture, 3, 4);
          res.mCmd->graphicsState().setTopology(p.mTopology);
          res.mCmd->graphicsState().setCullMode(
            p.mMaterial->mDoubleSided ? vk::CullModeFlagBits::eNone : vk::CullModeFlagBits::eBack);
          res.mCmd->drawIndexed(p.mIndexCount, 1, p.mIndexOffset, 0, 0);
        }
      }
    }

    drawNodes(n->mChildren, viewMatrix, modelMatrix, doAlphaBlending, emptySkinDynamicOffset, res);
  }
};

int main(int argc, char* argv[]) {

  struct {
    std::string mModelFile   = "data/models/DamagedHelmet.glb";
    std::string mSkyboxFile  = "data/textures/sunset_fairway_1k.hdr";
    std::string mTexChannels = "rgb";
    int         mAnimation   = 0;
    bool        mNoSkins     = false;
    bool        mNoTextures  = false;
    bool        mPrintHelp   = false;
  } options;

  // clang-format off
  Illusion::Core::CommandLineOptions args("Simple loader for GLTF files.");
  args.addOption({"-m",  "--model"},       &options.mModelFile,  "GLTF model (.gltf or .glb)");
  args.addOption({"-e",  "--environment"}, &options.mSkyboxFile, "Skybox image (in equirectangular projection)");
  args.addOption({"-a",  "--animation"},   &options.mAnimation,  "Index of the animation to play. Default: 0, Use -1 to disable animations.");
  args.addOption({"-ns", "--no-skins"},    &options.mNoSkins,    "Disable loading of skins");
  args.addOption({"-nt", "--no-textures"}, &options.mNoTextures, "Disable loading of textures");
  args.addOption({"-h",  "--help"},        &options.mPrintHelp,  "Print this help");
  // clang-format on

  args.parse(argc, argv);

  if (options.mPrintHelp) {
    args.printHelp();
    return 0;
  }

  auto engine = Illusion::Graphics::Engine::create("Simple GLTF Loader");
  auto device = Illusion::Graphics::Device::create(engine->getPhysicalDevice());
  auto window = Illusion::Graphics::Window::create(engine, device);

  Illusion::Graphics::GltfModel::OptionFlags modelOptions;
  if (options.mAnimation >= 0) {
    modelOptions |= Illusion::Graphics::GltfModel::OptionFlagBits::eAnimations;
  }
  if (!options.mNoSkins) {
    modelOptions |= Illusion::Graphics::GltfModel::OptionFlagBits::eSkins;
  }
  if (!options.mNoTextures) {
    modelOptions |= Illusion::Graphics::GltfModel::OptionFlagBits::eTextures;
  }

  auto model = Illusion::Graphics::GltfModel::create(device, options.mModelFile, modelOptions);
  model->printInfo();

  auto      modelBBox   = model->getRoot().getBoundingBox();
  float     modelSize   = glm::length(modelBBox.mMin - modelBBox.mMax);
  glm::vec3 modelCenter = (modelBBox.mMin + modelBBox.mMax) * 0.5f;
  glm::mat4 modelMatrix = glm::scale(glm::vec3(1.f / modelSize));
  modelMatrix           = glm::translate(modelMatrix, -modelCenter);

  auto brdflut = Illusion::Graphics::TextureUtils::createBRDFLuT(device, 128);
  auto skybox  = Illusion::Graphics::TextureUtils::createCubemapFrom360PanoramaFile(
    device, options.mSkyboxFile, 1024);
  auto prefilteredIrradiance =
    Illusion::Graphics::TextureUtils::createPrefilteredIrradianceCubemap(device, 64, skybox);
  auto prefilteredReflection =
    Illusion::Graphics::TextureUtils::createPrefilteredReflectionCubemap(device, 128, skybox);

  auto pbrShader = Illusion::Graphics::ShaderProgram::createFromFiles(device,
    {"data/shaders/SimpleGltfShader.vert", "data/shaders/SimpleGltfShader.frag"}, {"SkinUniforms"});

  auto skyShader = Illusion::Graphics::ShaderProgram::createFromFiles(
    device, {"data/shaders/Quad.vert", "data/shaders/Skybox.frag"});

  auto uboAlignment =
    engine->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

  Illusion::Core::RingBuffer<FrameResources, 2> frameResources{
    FrameResources(device, uboAlignment), FrameResources(device, uboAlignment)};

  glm::vec3 cameraPolar(0.f, 0.f, 1.5f);

  window->sOnMouseEvent.connect([&cameraPolar, &window](Illusion::Input::MouseEvent const& e) {
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

  Illusion::Core::Timer timer;

  while (!window->shouldClose()) {

    window->processInput();

    if (options.mAnimation >= 0 && options.mAnimation < model->getAnimations().size()) {
      auto const& anim         = model->getAnimations()[options.mAnimation];
      float modelAnimationTime = std::fmod((float)timer.getElapsed(), anim->mEnd - anim->mStart);
      modelAnimationTime += anim->mStart;
      model->setAnimationTime(options.mAnimation, modelAnimationTime);
    }

    model->update();

    auto& res = frameResources.next();

    device->waitForFences(*res.mRenderFinishedFence, true, ~0);
    device->resetFences(*res.mRenderFinishedFence);

    res.mCmd->reset();
    res.mCmd->begin();

    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setViewports(
      {{glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f}});

    CameraUniforms camera;
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

    res.mCmd->beginRenderPass(res.mRenderPass);

    res.mCmd->bindingState().setUniformBuffer(
      res.mUniformBuffer->getBuffer(), sizeof(CameraUniforms), 0, 0, 0);

    res.mCmd->setShaderProgram(skyShader);
    res.mCmd->bindingState().setTexture(skybox, 1, 0);
    res.mCmd->graphicsState().setDepthTestEnable(false);
    res.mCmd->graphicsState().setDepthWriteEnable(false);
    res.mCmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleStrip);
    res.mCmd->graphicsState().setVertexInputAttributes({});
    res.mCmd->graphicsState().setVertexInputBindings({});

    res.mCmd->draw(4);

    res.mCmd->bindingState().reset(1);

    res.mCmd->setShaderProgram(pbrShader);
    res.mCmd->bindingState().setTexture(brdflut, 1, 0);
    res.mCmd->bindingState().setTexture(prefilteredIrradiance, 1, 1);
    res.mCmd->bindingState().setTexture(prefilteredReflection, 1, 2);
    res.mCmd->graphicsState().setDepthTestEnable(true);
    res.mCmd->graphicsState().setDepthWriteEnable(true);
    res.mCmd->graphicsState().setVertexInputAttributes(
      Illusion::Graphics::GltfModel::getVertexInputAttributes());
    res.mCmd->graphicsState().setVertexInputBindings(
      Illusion::Graphics::GltfModel::getVertexInputBindings());

    res.mCmd->bindVertexBuffers(0, {model->getVertexBuffer()});
    res.mCmd->bindIndexBuffer(model->getIndexBuffer(), 0, vk::IndexType::eUint32);

    SkinUniforms ubo;
    uint32_t     emptySkinDynamicOffset = res.mUniformBuffer->addData(ubo);

    drawNodes(model->getRoot().mChildren, camera.mViewMatrix, modelMatrix, false,
      emptySkinDynamicOffset, res);
    drawNodes(model->getRoot().mChildren, camera.mViewMatrix, modelMatrix, true,
      emptySkinDynamicOffset, res);

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
