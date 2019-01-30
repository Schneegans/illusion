////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Device.hpp>
#include <Illusion/Graphics/FrameGraph.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto instance = Illusion::Graphics::Instance::create("DeferredRenderingDemo");
  auto device   = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create("Window", instance, device);

  auto index = Illusion::Graphics::FrameResourceIndex::create(3);
  auto graph = Illusion::Graphics::FrameGraph::create("FrameGraph", device, index);

  // create resources ------------------------------------------------------------------------------
  auto& albedo = graph->createResource().setName("albedo").setFormat(vk::Format::eR8G8B8A8Unorm);
  auto& normal = graph->createResource().setName("normal").setFormat(vk::Format::eR8G8B8A8Unorm);
  auto& depth  = graph->createResource().setName("depth").setFormat(vk::Format::eD32Sfloat);
  auto& hdr    = graph->createResource().setName("hdr").setFormat(vk::Format::eR32G32B32A32Sfloat);

  // create passes ---------------------------------------------------------------------------------
  using Access = Illusion::Graphics::FrameGraph::ResourceAccess;

  auto clearColor = vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}});
  auto clearDepth = vk::ClearDepthStencilValue(1.f, 0u);

  // clang-format off
  auto& gbuffer = graph->createPass()
    .setName("gbuffer")
    .assignResource(albedo, clearColor)
    .assignResource(normal, clearColor)
    .assignResource(depth, clearDepth)
    .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
      Illusion::Core::Logger::message() << "Record gbuffer pass!" << std::endl;
    });

  auto& lighting = graph->createPass()
    .setName("lighting")
    .assignResource(albedo, Access::eReadOnly)
    .assignResource(normal, Access::eReadOnly)
    .assignResource(depth, Access::eReadOnly)
    .assignResource(hdr, Access::eWriteOnly)
    .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
      Illusion::Core::Logger::message() << "Record lighting pass!" << std::endl;
    });

  auto& transparencies = graph->createPass()
    .setName("transparencies")
    .assignResource(depth, Access::eLoad)
    .assignResource(hdr, Access::eLoadWrite)
    .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
      Illusion::Core::Logger::message() << "Record transparencies pass!" << std::endl;
    });

  auto& tonemapping = graph->createPass()
    .setName("tonemapping")
    .assignResource(hdr, Access::eReadWrite)
    .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
      Illusion::Core::Logger::message() << "Record tonemapping pass!" << std::endl;
    });
  // clang-format on

  graph->setOutput(window, tonemapping, hdr);

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
    index->step();

    try {
      graph->process();
      // graph->process(Illusion::Graphics::FrameGraph::ProcessingFlagBits::eParallelSubpassRecording);
    } catch (std::runtime_error const& e) {
      Illusion::Core::Logger::error() << e.what() << std::endl;
    }

    // Prevent the GPU from over-heating :)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    device->waitIdle();
    return 0;
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
