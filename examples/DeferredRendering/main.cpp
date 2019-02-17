////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/CommandLineOptions.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Device.hpp>
#include <Illusion/Graphics/FrameGraph.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <thread>

#include "Floor.hpp"
#include "Lights.hpp"
#include "ToneMapping.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  struct {
    uint32_t mLightCount = 20;
    bool     mPrintHelp  = false;
  } options;

  // clang-format off
  Illusion::Core::CommandLineOptions args("Deferred Rendering with Vulkan.");
  args.addOption({"-h",  "--help"},   &options.mPrintHelp,  "Print this help");
  args.addOption({"-l",  "--lights"}, &options.mLightCount, "Number of light sources");
  args.addOption({"-t",  "--trace"},  &Illusion::Core::Logger::enableTrace, "Print trace output");
  // clang-format on

  args.parse(argc, argv);

  if (options.mPrintHelp) {
    args.printHelp();
    return 0;
  }

  auto instance = Illusion::Graphics::Instance::create("DeferredRenderingDemo");
  auto device   = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create("Window", instance, device);

  auto frameIndex = Illusion::Graphics::FrameResourceIndex::create(3);
  auto graph      = Illusion::Graphics::FrameGraph::create("FrameGraph", device, frameIndex);

  // create shaders and other resources ------------------------------------------------------------
  Lights      lights(device, frameIndex, options.mLightCount);
  Floor       floor(device);
  ToneMapping toneMapping(device);

  // create frame graph resources ------------------------------------------------------------------
  auto& albedo = graph->createResource().setName("albedo").setFormat(vk::Format::eR8G8B8A8Unorm);
  auto& normal = graph->createResource().setName("normal").setFormat(vk::Format::eR8G8B8A8Unorm);
  auto& emit   = graph->createResource().setName("emit").setFormat(vk::Format::eR16G16B16A16Sfloat);
  auto& depth  = graph->createResource().setName("depth").setFormat(vk::Format::eD32Sfloat);
  auto& hdr    = graph->createResource().setName("hdr").setFormat(vk::Format::eR16G16B16A16Sfloat);

  // create passes ---------------------------------------------------------------------------------
  using Access = Illusion::Graphics::FrameGraph::AccessFlagBits;

  auto clearColor = vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}});
  auto clearDepth = vk::ClearDepthStencilValue(1.f, 0u);

  auto& gbufferPass = graph->createPass();
  gbufferPass.setName("gbuffer");
  gbufferPass.addColorAttachment(albedo, Access::eWrite, clearColor);
  gbufferPass.addColorAttachment(normal, Access::eWrite, clearColor);
  gbufferPass.addColorAttachment(emit, Access::eWrite, clearColor);
  gbufferPass.addDepthAttachment(depth, Access::eWrite, clearDepth);
  gbufferPass.setProcessCallback([&lights, &floor](auto const& cmd, auto const&) {
    floor.draw(cmd);
    lights.draw(cmd);
  });

  auto& lightingPass = graph->createPass();
  lightingPass.setName("lighting");
  lightingPass.addColorAttachment(albedo, Access::eRead);
  lightingPass.addColorAttachment(normal, Access::eRead);
  lightingPass.addColorAttachment(emit, Access::eRead);
  lightingPass.addColorAttachment(depth, Access::eRead);
  lightingPass.addColorAttachment(hdr, Access::eWrite, clearColor);
  lightingPass.setProcessCallback([&lights](auto const& cmd, auto const& inputAttachments) {
    lights.doShading(cmd, inputAttachments);
  });

  auto& toneMappingPass = graph->createPass();
  toneMappingPass.setName("toneMapping");
  toneMappingPass.addColorAttachment(hdr, Access::eRead | Access::eWrite);
  toneMappingPass.setProcessCallback([&toneMapping](auto const& cmd, auto const& inputAttachments) {
    toneMapping.draw(cmd, inputAttachments);
  });

  // auto& debug = graph->createPass()
  //   .setName("debug")
  //   .addColorAttachment(hdr, Access::eLoad | Access::eWrite)
  //   .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
  //     Illusion::Core::Logger::message() << "Record debug pass!" << std::endl;
  //   });

  graph->setOutput(window, toneMappingPass, hdr);

  // do one rendering step -------------------------------------------------------------------------

  // Use a timer to get the current system time at each frame.
  Illusion::Core::Timer timer;

  // Then we open our window.
  window->open();

  // And start the application loop.
  while (!window->shouldClose()) {

    // This will trigger re-creations of the swapchain and make sure that window->shouldClose()
    // actually returns true when the user closed the window.
    window->update();

    // First, we increase our frame index. After this call, the frameResources will return their
    // next ring bffer entry.
    frameIndex->step();

    float time = timer.getElapsed();

    // Compute a projection matrix and write the data to our uniform buffer.
    glm::mat4 projection = glm::perspectiveZO(glm::radians(60.f),
        static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
        0.1f, 100.0f);
    projection[1][1] *= -1; // flip for y-down

    // Compute a view matrix based on the simulation time (this makes the scene spin). Then
    // upload this matrix via push constants.
    glm::mat4 view(1.f);
    view = glm::translate(view, glm::vec3(0, 0, -3));
    view = glm::rotate(view, -time * 0.5f, glm::vec3(0, 1, 0));

    lights.update(time, projection * view);
    floor.update(projection * view);

    try {
      graph->process();
    } catch (std::runtime_error const& e) {
      Illusion::Core::Logger::error() << e.what() << std::endl;
    }
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
