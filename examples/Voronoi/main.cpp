////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/CommandLine.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CoherentBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Device.hpp>
#include <Illusion/Graphics/FrameGraph.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Window.hpp>

#include "VoronoiGenerator.hpp"

#include <chrono>
#include <random>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  struct {
    uint32_t mPointCount = 20;
    bool     mPrintHelp  = false;
  } options;

  // clang-format off
  Illusion::Core::CommandLine args("Voronoi Diagram Rendering with Vulkan.");
  args.addArgument({"-h", "--help"},   &options.mPrintHelp,  "Print this help");
  args.addArgument({"-p", "--points"}, &options.mPointCount, "Number of points");
  args.addArgument({"-t", "--trace"},  &Illusion::Core::Logger::enableTrace, "Print trace output");
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

  VoronoiGenerator voronoi;

  auto pointShader = Illusion::Graphics::Shader::createFromFiles(
      "PointShader", device, {"data/Voronoi/Point.vert", "data/Voronoi/Simple.frag"});

  auto lineShader = Illusion::Graphics::Shader::createFromFiles(
      "LineShader", device, {"data/Voronoi/Line.vert", "data/Voronoi/Simple.frag"});

  Illusion::Graphics::FrameResource<Illusion::Graphics::CoherentBufferPtr> positionBuffer(
      frameIndex, [=](uint32_t index) {
        return Illusion::Graphics::CoherentBuffer::create("PositionBuffer " + std::to_string(index),
            device, sizeof(glm::vec2) * options.mPointCount,
            vk::BufferUsageFlagBits::eVertexBuffer);
      });

  Illusion::Graphics::FrameResource<Illusion::Graphics::CoherentBufferPtr> edgeBuffer(
      frameIndex, [=](uint32_t index) {
        return Illusion::Graphics::CoherentBuffer::create("EdgeBuffer " + std::to_string(index),
            device, sizeof(glm::vec2) * options.mPointCount * 2 * 5,
            vk::BufferUsageFlagBits::eVertexBuffer);
      });

  Illusion::Graphics::FrameResource<Illusion::Graphics::CoherentBufferPtr> triangulationBuffer(
      frameIndex, [=](uint32_t index) {
        return Illusion::Graphics::CoherentBuffer::create(
            "TriangulationBuffer " + std::to_string(index), device,
            sizeof(glm::vec2) * options.mPointCount * 2 * 5,
            vk::BufferUsageFlagBits::eVertexBuffer);
      });

  auto& color = graph->createResource().setName("color").setFormat(vk::Format::eB8G8R8A8Unorm);
  color.setSamples(vk::SampleCountFlagBits::e16);
  auto& renderPass = graph->createPass();
  renderPass.setName("drawing");
  renderPass.addColorAttachment(color, Illusion::Graphics::FrameGraph::AccessFlagBits::eWrite,
      vk::ClearColorValue(std::array<float, 4>{{1.f, 1.f, 1.f, 1.f}}));
  renderPass.setProcessCallback([&](auto const& cmd, auto const&) {
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> edges;
    std::vector<glm::vec2> triangulation;

    for (auto const& s : voronoi.getSites()) {
      positions.emplace_back(s.x, s.y);
    }

    for (auto const& s : voronoi.getEdges()) {
      edges.emplace_back(s.first.x, s.first.y);
      edges.emplace_back(s.second.x, s.second.y);
    }

    for (auto const& s : voronoi.getTriangulation()) {
      triangulation.emplace_back(s.first.x, s.first.y);
      triangulation.emplace_back(s.second.x, s.second.y);
    }

    positionBuffer.current()->updateData(
        reinterpret_cast<const uint8_t*>(&positions[0]), sizeof(glm::vec2) * positions.size(), 0);
    edgeBuffer.current()->updateData(
        reinterpret_cast<const uint8_t*>(&edges[0]), sizeof(glm::vec2) * edges.size(), 0);
    triangulationBuffer.current()->updateData(reinterpret_cast<const uint8_t*>(&triangulation[0]),
        sizeof(glm::vec2) * triangulation.size(), 0);

    cmd->graphicsState().setVertexInputBindings(
        {{0, sizeof(glm::vec2), vk::VertexInputRate::eVertex}});

    cmd->graphicsState().setRasterizationSamples(vk::SampleCountFlagBits::e16);
    cmd->graphicsState().setVertexInputAttributes({{0, 0, vk::Format::eR32G32Sfloat, 0}});
    cmd->graphicsState().setTopology(vk::PrimitiveTopology::eLineList);
    cmd->setShader(lineShader);

    cmd->graphicsState().setLineWidth(2);
    cmd->specialisationState().setFloatConstant(0, 1.0f);
    cmd->specialisationState().setFloatConstant(1, 0.6f);
    cmd->specialisationState().setFloatConstant(2, 0.6f);
    cmd->bindVertexBuffers(0, {edgeBuffer.current()->getBuffer()});
    cmd->draw(voronoi.getEdges().size() * 2);

    cmd->graphicsState().setLineWidth(1);
    cmd->specialisationState().setFloatConstant(0, 0.7f);
    cmd->specialisationState().setFloatConstant(1, 0.7f);
    cmd->specialisationState().setFloatConstant(2, 0.7f);
    cmd->bindVertexBuffers(0, {triangulationBuffer.current()->getBuffer()});
    cmd->draw(voronoi.getTriangulation().size() * 2);

    cmd->graphicsState().setTopology(vk::PrimitiveTopology::ePointList);
    cmd->setShader(pointShader);

    cmd->specialisationState().setFloatConstant(0, 0.5f);
    cmd->specialisationState().setFloatConstant(1, 0.5f);
    cmd->specialisationState().setFloatConstant(2, 0.5f);
    cmd->specialisationState().setFloatConstant(3, 5.f);
    cmd->bindVertexBuffers(0, {positionBuffer.current()->getBuffer()});
    cmd->draw(voronoi.getSites().size());
  });

  graph->setOutput(window, renderPass, color);

  std::default_random_engine random;
  random.seed(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<float> positionGenerator(-1.f, 1.f);
  std::uniform_real_distribution<float> velocityGenerator(-0.03f, 0.03f);

  std::vector<Site> positions;
  for (uint32_t i(0); i < options.mPointCount; ++i) {
    positions.emplace_back(positionGenerator(random), positionGenerator(random), i);
  }

  std::vector<glm::vec2> velocities(options.mPointCount);
  for (auto& velocity : velocities) {
    velocity.x = velocityGenerator(random);
    velocity.y = velocityGenerator(random);
  }

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
    timer.reset();

    // Modify velocities over time.
    for (auto& velocity : velocities) {
      velocity.x += time * velocityGenerator(random) * 10;
      velocity.y += time * velocityGenerator(random) * 10;
    }

    // Update positions.
    for (size_t i(0); i < positions.size(); ++i) {
      positions[i].x += velocities[i].x * time * 0.1f;
      positions[i].y += velocities[i].y * time * 0.1f;

      if (positions[i].x > 1) {
        positions[i].x  = 1;
        velocities[i].x = -velocities[i].x;
      } else if (positions[i].x < -1) {
        positions[i].x  = -1;
        velocities[i].x = -velocities[i].x;
      }
      if (positions[i].y > 1) {
        positions[i].y  = 1;
        velocities[i].y = -velocities[i].y;
      } else if (positions[i].y < -1) {
        positions[i].y  = -1;
        velocities[i].y = -velocities[i].y;
      }
    }

    voronoi.parse(positions);

    try {
      graph->process();
    } catch (std::runtime_error const& e) {
      Illusion::Core::Logger::error() << e.what() << std::endl;
    }

    // Prevent the GPU from over-heating :)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
