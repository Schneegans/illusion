////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Timer.hpp>
#include <Illusion/Graphics/CoherentBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/FrameResource.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/LazyRenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// When compared to the ShaderSandbox example, this example is a bit more involved in several     //
// ways. On the one hand, we use actual vertex and index buffers, on the other, we use a set of   //
// per-frame resources so that we can start recording the next frame while the last one is still  //
// being processed.                                                                               //
// The following struct contains all resources we will need for one frame. While one frame is     //
// processed by the GPU, we will acquire an instance of PerFrame and work with that one. We will  //
// store the PerFrame in a ring-buffer and re-use older PerFrames after some time when the GPU is //
// likely to be finished processing it anyways.                                                   //
// The PerFrame contains a command buffer, a render pass, a uniform buffer (for the projection    //
// matrix), a semaphore indicating when rendering has finished (the frame buffer is ready for     //
// presentation) and a fence telling us when the PerFrame is ready to be re-used.                 //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct PerFrame {
  PerFrame() = default;
  PerFrame(uint32_t index, Illusion::Graphics::DevicePtr const& device)
      : mCmd(Illusion::Graphics::CommandBuffer::create(
            "CommandBuffer " + std::to_string(index), device))
      , mRenderPass(Illusion::Graphics::LazyRenderPass::create(
            "RenderPass " + std::to_string(index), device))
      , mUniformBuffer(
            Illusion::Graphics::CoherentBuffer::create("CoherentBuffer " + std::to_string(index),
                device, sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer))
      , mFrameFinishedFence(device->createFence("RenderFinished " + std::to_string(index)))
      , mRenderFinishedSemaphore(
            device->createSemaphore("FrameFinished " + std::to_string(index))) {

    // In addition to a color buffer we will need a depth buffer for depth testing.
    mRenderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
    mRenderPass->addAttachment(vk::Format::eD32Sfloat);

    // The indices are provided as a triangle list
    mCmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleList);

    // Here we define what kind of vertex buffers will be bound. The vertex data (positions, normals
    // and texture coordinates) actually comes from three different vertex buffer objects.
    mCmd->graphicsState().setVertexInputBindings(
        {{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
            {1, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
            {2, sizeof(glm::vec2), vk::VertexInputRate::eVertex}});

    // Here we define which vertex attribute comes from which vertex buffer.
    mCmd->graphicsState().setVertexInputAttributes({{0, 0, vk::Format::eR32G32B32Sfloat, 0},
        {1, 1, vk::Format::eR32G32B32Sfloat, 0}, {2, 2, vk::Format::eR32G32Sfloat, 0}});
  }

  Illusion::Graphics::CommandBufferPtr  mCmd;
  Illusion::Graphics::LazyRenderPassPtr mRenderPass;
  Illusion::Graphics::CoherentBufferPtr mUniformBuffer;
  vk::FencePtr                          mFrameFinishedFence;
  vk::SemaphorePtr                      mRenderFinishedSemaphore;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {

  // Enable trace output. This is useful to see Vulkan object lifetime.
  Illusion::Core::Logger::enableTrace = true;

  // Then we start setting up our Vulkan resources.
  auto instance = Illusion::Graphics::Instance::create("TexturedCubeDemo");
  auto device   = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create("Window", instance, device);

  // Here we load the texture. This supports many file formats (those supported by gli and stb).
  auto texture = Illusion::Graphics::Texture::createFromFile(
      "BoxTexture", device, "data/TexturedCube/textures/box.dds");

  // Load the shader. You can have a look at the files for some more comments on how they work.
  auto shader = Illusion::Graphics::Shader::createFromFiles("CubeShader", device,
      {"data/TexturedCube/shaders/Cube.vert", "data/TexturedCube/shaders/Cube.frag"});

  // Here we create our three vertex buffers and one index buffer.
  // clang-format off
  const std::array<glm::vec3, 26> POSITIONS = {
    glm::vec3( 1, -1, 1),  glm::vec3(-1, -1, -1), glm::vec3( 1, -1, -1), glm::vec3(-1,  1, -1),
    glm::vec3( 1,  1,  1), glm::vec3( 1,  1, -1), glm::vec3( 1,  1, -1), glm::vec3( 1, -1,  1),
    glm::vec3( 1, -1, -1), glm::vec3( 1,  1,  1), glm::vec3(-1, -1,  1), glm::vec3( 1, -1,  1),
    glm::vec3(-1, -1,  1), glm::vec3(-1,  1, -1), glm::vec3(-1, -1, -1), glm::vec3( 1, -1, -1),
    glm::vec3(-1,  1, -1), glm::vec3( 1,  1, -1), glm::vec3(-1, -1,  1), glm::vec3(-1,  1,  1),
    glm::vec3( 1,  1, -1), glm::vec3( 1,  1,  1), glm::vec3( 1, -1,  1), glm::vec3(-1,  1,  1),
    glm::vec3(-1,  1,  1), glm::vec3(-1, -1, -1)
  };

  const std::array<glm::vec3, 26> NORMALS = {
    glm::vec3( 0, -1,  0), glm::vec3( 0, -1,  0), glm::vec3( 0, -1,  0), glm::vec3( 0,  1,  0),
    glm::vec3( 0,  1,  0), glm::vec3( 0,  1,  0), glm::vec3( 1,  0,  0), glm::vec3( 1,  0,  0),
    glm::vec3( 1,  0,  0), glm::vec3( 0,  0,  1), glm::vec3( 0,  0,  1), glm::vec3( 0,  0,  1),
    glm::vec3(-1,  0,  0), glm::vec3(-1,  0,  0), glm::vec3(-1,  0,  0), glm::vec3( 0,  0, -1),
    glm::vec3( 0,  0, -1), glm::vec3( 0,  0, -1), glm::vec3( 0, -1,  0), glm::vec3( 0,  1,  0),
    glm::vec3( 1,  0,  0), glm::vec3( 1,  0,  0), glm::vec3( 1,  0,  0), glm::vec3( 0,  0,  1),
    glm::vec3(-1,  0,  0), glm::vec3( 0,  0, -1)
  };

  const std::array<glm::vec2, 26> TEXCOORDS = {
    glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1),
    glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 0), 
    glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(0, 1), 
    glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(1, 1), 
    glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), 
    glm::vec2(1, 1)
  };

  const std::array<uint32_t, 36> INDICES = {
    0, 1,  2, 3, 4,  5, 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
    0, 18, 1, 3, 19, 4, 20, 21, 22, 9, 23, 10, 12, 24, 13, 15, 25, 16,
  };
  // clang-format on

  auto positionBuffer = device->createVertexBuffer("CubePositions", POSITIONS);
  auto normalBuffer   = device->createVertexBuffer("CubeNormals", NORMALS);
  auto texcoordBuffer = device->createVertexBuffer("CubeTexcoords", TEXCOORDS);
  auto indexBuffer    = device->createIndexBuffer("CubeIndices", INDICES);

  // Here begin the interesting bits. In Illusion, per-frame resources are implemented with two
  // classes: The FrameResourceIndex keeps track of an index (a simple uint32_t) in a ring-buffer
  // like fashion. That means it can be increased with its step() method, but it will be reset to
  // zero once its allowed maximum is reached. This index will then be used by the second class, the
  // actual FrameResource. This is a template class which holds whatever you like in an actual
  // ring-buffer.
  // We use only a ring buffer size of two here in this example but you could change this number to
  // whatever you like. Three or four could improve the performance in some cases, but this will use
  // more memory and could also lead to some input lag. Usually two or three will be a could choice.
  auto frameIndex = Illusion::Graphics::FrameResourceIndex::create(2);

  // The actual FrameResource template wraps anything you like in a ring-buffer internally. The
  // index into its ring buffer is defined by the FrameResourceIndex which is passed as first
  // parameter to its contructor. The second argument is a function (lambda) which is invoked once
  // for each ring buffer entry, thus serving as a factory. It should return an instance of the
  // wrapped type. So in this example, the lambda is executed two times, once for each ring buffer
  // index.
  Illusion::Graphics::FrameResource<PerFrame> perFrame(
      frameIndex, [=](uint32_t index) { return PerFrame(index, device); });

  // Use a timer to get the current system time at each frame.
  Illusion::Core::Timer timer;

  // Then we open our window.
  window->open();

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
    device->waitForFence(res.mFrameFinishedFence);
    device->resetFence(res.mFrameFinishedFence);

    // Get the current time for animations.
    float time = timer.getElapsed();

    // As we are re-recording our command buffer, we have to reset it before starting to record new
    // commands.
    res.mCmd->reset();
    res.mCmd->begin();

    // Set the shader to be used.
    res.mCmd->setShader(shader);

    // Adapt the render pass and viewport sizes.
    res.mRenderPass->setExtent(window->pExtent.get());
    res.mCmd->graphicsState().setViewports({{glm::vec2(window->pExtent.get())}});

    // Compute a projection matrix and write the data to our uniform buffer.
    glm::mat4 projection = glm::perspectiveZO(glm::radians(60.f),
        static_cast<float>(window->pExtent.get().x) / static_cast<float>(window->pExtent.get().y),
        0.1f, 100.0f);
    projection[1][1] *= -1; // flip for y-down
    res.mUniformBuffer->updateData(projection);

    // Bind the uniform buffer to descriptor set 0
    res.mCmd->bindingState().setUniformBuffer(
        res.mUniformBuffer->getBuffer(), sizeof(glm::mat4), 0, 0, 0);

    // Bind the texture to descriptor set 1
    res.mCmd->bindingState().setTexture(texture, 1, 0);

    // The color and depth our framebuffer attachments will be cleared to.
    std::vector<vk::ClearValue> clearValues;
    clearValues.emplace_back(vk::ClearColorValue(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}}));
    clearValues.emplace_back(vk::ClearDepthStencilValue(1.f, 0u));

    // Begin our render pass.
    res.mCmd->beginRenderPass(res.mRenderPass, clearValues);

    // Compute a modelView matrix based on the simulation time (this makes the cube spin). Then
    // upload this matrix via push constants.
    glm::mat4 modelView(1.f);
    modelView = glm::translate(modelView, glm::vec3(0, 0, -3));
    modelView = glm::rotate(modelView, -time * 0.5f, glm::vec3(0, 1, 0));
    modelView = glm::rotate(modelView, time * 0.3f, glm::vec3(1, 0, 0));
    res.mCmd->pushConstants(modelView);

    // Bind the three vertex buffers and the index buffer.
    res.mCmd->bindVertexBuffers(0, {positionBuffer, normalBuffer, texcoordBuffer});
    res.mCmd->bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

    // Do the actual drawing.
    res.mCmd->drawIndexed(static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);

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
        res.mFrameFinishedFence);

    // Prevent the GPU from over-heating :)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
