////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the most simple example for Vulkan rendering. A pre-recordeded command buffer is used  //
// every frame to draw a simple Triangle. No vertex information is passed to the rendering        //
// pipeline, instead hard-coded position and color values are accessed with gl_VertexIndex in the //
// vertex shader.                                                                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {

  // Enable trace output. This is useful to see Vulkan object lifetime.
  Illusion::Core::Logger::enableTrace = true;

  // These three things every application will need: an instance, a device and a window.
  auto instance = Illusion::Graphics::Instance::create("Triangle Demo");
  auto device   = Illusion::Graphics::Device::create(instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create(instance, device);

  // Create a shader program. The shader stages a deduced from the file extensions.
  auto shader = Illusion::Graphics::Shader::createFromFiles(
      device, {"data/shaders/Triangle.vert", "data/shaders/Triangle.frag"});

  // All rendering is done inside an active render pass. This render pass creates an associated
  // frame buffer with the given attachments (a color buffer in this case). We use the window's
  // resolution for the render pass.
  auto renderPass = Illusion::Graphics::RenderPass::create(device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  // Now create and record the command buffer. In this example we use a pre-recorded command buffer
  // during rendering. Usually you will re-record the command buffer every frame.
  auto cmd = Illusion::Graphics::CommandBuffer::create(device);
  cmd->graphicsState().addViewport({glm::vec2(window->pExtent.get())}); // Set the viewport
  cmd->setShader(shader);                                               // Set the shader
  cmd->begin();                                                         // Begin recording
  cmd->beginRenderPass(renderPass);                                     // Begin our render pass
  cmd->draw(3);                                                         // Draw three vertices
  cmd->endRenderPass();                                                 // End our render pass
  cmd->end();                                                           // Finish recording

  // This semaphore will be signaled when rendering has finished and the frame buffer is ready to be
  // presented on our window.
  auto renderFinishedSemaphore = device->createSemaphore();

  // This fence will be signaled when the frame buffer has been blitted to the swapchain image and
  // we are ready to start the next frame.
  auto frameFinishedFence = device->createFence();

  // Now we open our window.
  window->open();

  // And start our main application loop.
  while (!window->shouldClose()) {

    // This will trigger re-creations of the swapchain and make sure that window->shouldClose()
    // actually returns true when the user closed the window.
    window->update();

    // Wait until the last frame has been fully processed. Afterwards, reset the fence so that we
    // can use it once more this frame.
    device->waitForFences(*frameFinishedFence);
    device->resetFences(*frameFinishedFence);

    // Our command buffer has been recorded already, so we can just submit it. Once it has been
    // processed, the renderFinishedSemaphore will be signaled.
    cmd->submit({}, {}, {*renderFinishedSemaphore});

    // Present the color attachment of the render pass on the window. This operation will wait for
    // the renderFinishedSemaphore and signal the frameFinishedFence so that we know when to start
    // the next frame.
    window->present(
        renderPass->getFramebuffer()->getImages()[0], renderFinishedSemaphore, frameFinishedFence);

    // Prevent the GPU from over-heating :)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
