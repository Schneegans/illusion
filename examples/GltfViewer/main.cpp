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
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This (when compared to the TexturedCube rather complex) example loads a glTF-file and displays //
// it using physically based shading. Both, roughness-metallic and secular-glossiness workflows   //
// are supported. It also loads animations and skins from the file. The loading if done by the    //
// Gltf::Model class of Illusion, the rendering is done in this example. Most of the official     //
// sample Models [1] are supported, especially the feature-test models render fine. You can also  //
// get many glTF models from sketchfab [2] for free and view them with this example. For image    //
// based lighting, 360-degree panorma files are used. You can directly load the files from [3].   //
//                                                                                                //
// [1] https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0                         //
// [2] https://sketchfab.com                                                                      //
// [3] https://hdrihaven.com                                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// This struct is uploaded to a uniform buffer.                                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CameraUniforms {
  glm::vec4 mPosition;
  glm::mat4 mViewMatrix;
  glm::mat4 mProjectionMatrix;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// This struct is very similar to the TexturedCube example. It contains all resources we will     //
// need for one frame. While one frame is processed by the GPU, we will acquire an instance of    //
// PerFrame and work with that one. We will store the PerFrame in a ring-buffer and re-use older  //
// PerFrames after some time when the GPU is likely to be finished processing it anyways.         //
// The PerFrame contains a command buffer, a render pass, a uniform buffer (for the               //
// CameraUniforms), a semaphore indicating when rendering has finished (the frame buffer is ready //
// for presentation) and a fence telling us when the PerFrame is ready to be re-used.             //
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

    // In addition to a color buffer we will need a depth buffer for depth testing.
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

  // The GltfViewer supports several command line options. For improved readability and some
  // scoping, we put all options into a struct.
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
  args.parse(argc, argv);
  // clang-format on

  // When mPrintHelp was set to true, we print a help message and exit.
  if (options.mPrintHelp) {
    args.printHelp();
    return 0;
  }

  // Then we start setting up our usual Vulkan resources.
  auto instance   = Illusion::Graphics::Instance::create("Simple GLTF Loader");
  auto device     = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());
  auto window     = Illusion::Graphics::Window::create("Window", instance, device);
  auto frameIndex = Illusion::Graphics::FrameResourceIndex::create(2);

  // We create the PerFrame instances. Have a look at the TexturedCube example for a more in-depth
  // explanation of per-frame resources in Illusion.
  Illusion::Graphics::FrameResource<PerFrame> perFrame(
      frameIndex, [=](uint32_t index) { return PerFrame(index, device); });

  // Then we create several textures. First is the BRDF-lookup texture for physically based shading.
  // Illusion uses compute shaders to calculate this texture at run-time.
  auto brdflut = Illusion::Graphics::Texture::createBRDFLuT("BRDFLuT", device, 128);

  // The skybox texture is a cubemap which is converted from a user-provides 360-degree panorma
  // file. The conversion is also done using compute shaders.
  auto skybox = Illusion::Graphics::Texture::createCubemapFrom360PanoramaFile(
      "SkyboxTexture", device, options.mSkyboxFile, 1024);

  // Then we create a prefiltered irradiance and reflection cubemap for this skybox. This is also
  // done with compute.
  auto prefilteredIrradiance = Illusion::Graphics::Texture::createPrefilteredIrradianceCubemap(
      "IrradianceTexture", device, 64, skybox);
  auto prefilteredReflection = Illusion::Graphics::Texture::createPrefilteredReflectionCubemap(
      "ReflectionTexture", device, 128, skybox);

  // This fullscreen shader is used to draw the skybox background.
  auto skyShader = Illusion::Graphics::Shader::createFromFiles("SkyboxShader", device,
      {"data/GltfViewer/shaders/Skybox.vert", "data/GltfViewer/shaders/Skybox.frag"});

  // Now the interesting part begins. Loading the glTF model! First we construct our
  // Gltf::LoadOptions based on the command line flags provided by the user.
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

  // Then we load the model. Note that this class is not from Illusion but from this example. The
  // actual loading happens in there. Maybe its a good dea to have a look at this class now :)
  auto model = GltfModel("GltfModel", device, options.mModelFile, loadOptions, frameIndex);

  // This is another class of this example which provides basic turntable-like navigation. If you
  // are interested, you may have alook at this class as well.
  Turntable turntable(window);

  // Before we start, we register a callback which toggles fullscreen when the user presses F11.
  window->sOnKeyEvent.connect([&window](Illusion::Input::KeyEvent const& e) {
    if (e.mType == Illusion::Input::KeyEvent::Type::ePress &&
        e.mKey == Illusion::Input::Key::eF11) {
      window->pFullscreen = !window->pFullscreen.get();
    }
    return true;
  });

  // Then we open our window.
  window->open();

  // Use a timer to get the current system time at each frame.
  Illusion::Core::Timer timer;

  // And start the application loop.
  while (!window->shouldClose()) {

    // This will trigger re-creations of the swapchain and make sure that window->shouldClose()
    // actually returns true when the user closed the window.
    window->update();

    // First, we increase our frame index. After this call, the PerFrame will return their
    // next ring bffer entry.
    frameIndex->step();

    // Then, we acquire the next PerFrame instance from our PerFrame.
    auto& res = perFrame.current();

    // Then we have to wait until the GPU has finished the last frame done with the current set of
    // frame resources. Usually this should return instantly because there was at least one frame in
    // between.
    device->waitForFence(res.mRenderFinishedFence);
    device->resetFence(res.mRenderFinishedFence);

    // Here we update the animation state of the glTF model.
    model.update(timer.getElapsed(), options.mAnimation);

    // As we are re-recording our command buffer, we have to reset it before starting to record new
    // commands.
    res.mCmd->reset();
    res.mCmd->begin();

    // Adapt the render pass and viewport sizes.
    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setViewports({{glm::vec2(window->pExtent.get())}});

    // Compute a projection matrix, a view matrix and the camera's position and write the data to
    // our uniform buffer.
    CameraUniforms camera{};
    camera.mProjectionMatrix = glm::perspectiveZO(glm::radians(50.f),
        static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
        0.01f, 10.0f);
    camera.mProjectionMatrix[1][1] *= -1;
    camera.mPosition   = turntable.getCameraPosition();
    camera.mViewMatrix = turntable.getViewMatrix();
    res.mUniformBuffer->updateData(camera);

    // Set the camera uniform buffer binding (set 0 and binding 0)
    res.mCmd->bindingState().setUniformBuffer(
        res.mUniformBuffer->getBuffer(), sizeof(CameraUniforms), 0, 0, 0);

    // The color and depth our framebuffer attachments will be cleared to.
    std::vector<vk::ClearValue> clearValues;
    clearValues.emplace_back(vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}}));
    clearValues.emplace_back(vk::ClearDepthStencilValue(1.f, 0u));

    // Begin our render pass.
    res.mCmd->beginRenderPass(res.mRenderPass, clearValues);

    // Now we draw the skybox background.
    res.mCmd->setShader(skyShader);
    res.mCmd->bindingState().setTexture(skybox, 1, 0);
    res.mCmd->graphicsState().setDepthTestEnable(false);
    res.mCmd->graphicsState().setDepthWriteEnable(false);
    res.mCmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleList);
    res.mCmd->graphicsState().setVertexInputAttributes({});
    res.mCmd->graphicsState().setVertexInputBindings({});
    res.mCmd->draw(3);

    // Then we draw the actual glTF model. Before we bind the texture required for image based
    // lighting to descriptor set 1.
    res.mCmd->bindingState().reset(1);
    res.mCmd->bindingState().setTexture(brdflut, 1, 0);
    res.mCmd->bindingState().setTexture(prefilteredIrradiance, 1, 1);
    res.mCmd->bindingState().setTexture(prefilteredReflection, 1, 2);
    res.mCmd->graphicsState().setDepthTestEnable(true);
    res.mCmd->graphicsState().setDepthWriteEnable(true);
    model.draw(res.mCmd, camera.mViewMatrix);

    // End the render pass and finish recording of the command buffer.
    res.mCmd->endRenderPass();
    res.mCmd->end();

    // Now we can just submit the command buffer. Once it has been processed, the
    // renderFinishedSemaphore will be signaled.
    res.mCmd->submit({}, {}, {res.mRenderFinishedSemaphore});

    // Present the color attachment of the render pass on the window. This operation will wait for
    // the renderFinishedSemaphore and signal the frameFinishedFence so that we know when to start
    // the next frame.
    window->present(res.mRenderPass->getAttachments()[0].mImage, res.mRenderFinishedSemaphore,
        res.mRenderFinishedFence);

    // Prevent the GPU from over-heating :)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
