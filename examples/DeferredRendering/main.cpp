////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/Device.hpp>
#include <Illusion/Graphics/FrameGraph.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto instance = Illusion::Graphics::Instance::create("Deferred Rendering Demo");
  auto device   = Illusion::Graphics::Device::create(instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create(instance, device);

  auto index = Illusion::Graphics::FrameResourceIndex::create(3);
  auto graph = Illusion::Graphics::FrameGraph::create(device, index);

  // create resources ------------------------------------------------------------------------------
  auto& albedo = graph->createResource().setName("albedo").setFormat(vk::Format::eR8G8B8Unorm);
  auto& normal = graph->createResource().setName("normal").setFormat(vk::Format::eR8G8B8Unorm);
  auto& depth  = graph->createResource().setName("depth").setFormat(vk::Format::eD32Sfloat);
  auto& hdr    = graph->createResource().setName("hdr").setFormat(vk::Format::eR32G32B32Sfloat);

  // create passes ---------------------------------------------------------------------------------
  using Usage  = Illusion::Graphics::FrameGraph::ResourceUsage;
  using Access = Illusion::Graphics::FrameGraph::ResourceAccess;

  auto clearColor = vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}});
  auto clearDepth = vk::ClearDepthStencilValue(1.f, 0u);

  graph->createPass()
      .setName("gbuffer")
      .assignResource(albedo, Usage::eColorAttachment, Access::eWriteOnly, clearColor)
      .assignResource(normal, Usage::eColorAttachment, Access::eWriteOnly, clearColor)
      .assignResource(depth, Usage::eDepthAttachment, Access::eWriteOnly, clearDepth)
      .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
        ILLUSION_MESSAGE << "Record gbuffer pass!" << std::endl;
      });

  graph->createPass()
      .setName("lighting")
      .assignResource(albedo, Usage::eColorAttachment, Access::eReadOnly)
      .assignResource(normal, Usage::eColorAttachment, Access::eReadOnly)
      .assignResource(depth, Usage::eColorAttachment, Access::eReadOnly)
      .assignResource(hdr, Usage::eColorAttachment, Access::eWriteOnly)
      .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
        ILLUSION_MESSAGE << "Record lighting pass!" << std::endl;
      });

  graph->createPass()
      .setName("transparencies")
      .assignResource(depth, Usage::eDepthAttachment, Access::eReadOnly)
      .assignResource(hdr, Usage::eColorAttachment, Access::eBlend)
      .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
        ILLUSION_MESSAGE << "Record transparencies pass!" << std::endl;
      });

  graph->createPass()
      .setName("tonemapping")
      .assignResource(hdr, Usage::eColorAttachment, Access::eReadOnly)
      .setOutputWindow(window)
      .setProcessCallback([](Illusion::Graphics::CommandBufferPtr const& cmd) {
        ILLUSION_MESSAGE << "Record tonemapping pass!" << std::endl;
      });

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

    graph->process();

    // Prevent the GPU from over-heating :)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
