////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/LazyRenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This example is based on the Triangle example. Not only one triangle is drawn but three of     //
// them. The position and color of each triangle is set via specialization constants. A pre-      //
// recordeded command buffer is used every frame to draw the triangles. In the background, three  //
// vk::Pipeline objects are created, one for each set of specialization constants. Those          //
// pipelines are then cached and used until the program exits.                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {

  // Enable trace output. This is useful to see Vulkan object lifetime. In this very example you can
  // see that three vk::Pipeline objects are created. One for each specialization.
  Illusion::Core::Logger::enableTrace = true;

  // These three things every application will need: an instance, a device and a window.
  auto instance = Illusion::Graphics::Instance::create("SpecializationConstantsDemo");
  auto device   = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create("Window", instance, device);

  // This shader contains five specialisation constants; two in the vertex stage and three in the
  // fragment stage.
  auto shader = Illusion::Graphics::Shader::createFromFiles("SpecializationConstantsShader", device,
      {"data/SpecializationConstants/Triangle.vert", "data/SpecializationConstants/Triangle.frag"});

  // Create a RenderPass. Same procedure as in the Triangle exmaple.
  auto renderPass = Illusion::Graphics::LazyRenderPass::create("RenderPass", device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  // The color our framebuffer attachment will be cleared to.
  vk::ClearColorValue clearColor(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}});

  // Now create and record the command buffer. In this example we use a pre-recorded command buffer
  // during rendering. Usually you will re-record the command buffer every frame.
  auto cmd = Illusion::Graphics::CommandBuffer::create("CommandBuffer", device);
  cmd->graphicsState().addViewport({glm::vec2(window->pExtent.get())}); // Set the viewport
  cmd->setShader(shader);                                               // Set the shader
  cmd->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);         // Begin recording
  cmd->beginRenderPass(renderPass, {clearColor});                       // Begin our render pass

  // Now we draw three time with the same shader. However, we change the specialization constants in
  // between. This results in three different vk::Pipeline objects being created in the background.
  // Specialization constants 0 and 1 are used for the position of the Triangles, constants 2, 3,
  // and 4 determine the color.

  // Red Triangle.
  cmd->specialisationState().setFloatConstant(0, -0.5f);
  cmd->specialisationState().setFloatConstant(1, -0.3f);
  cmd->specialisationState().setFloatConstant(2, 1.f);
  cmd->specialisationState().setFloatConstant(3, 0.2f);
  cmd->specialisationState().setFloatConstant(4, 0.2f);
  cmd->draw(3);

  // Blue Triangle.
  cmd->specialisationState().setFloatConstant(0, 0.5f);
  cmd->specialisationState().setFloatConstant(1, -0.3f);
  cmd->specialisationState().setFloatConstant(2, 0.2f);
  cmd->specialisationState().setFloatConstant(3, 0.2f);
  cmd->specialisationState().setFloatConstant(4, 1.f);
  cmd->draw(3);

  // Green Triangle.
  cmd->specialisationState().setFloatConstant(0, 0.f);
  cmd->specialisationState().setFloatConstant(1, 0.4f);
  cmd->specialisationState().setFloatConstant(2, 0.2f);
  cmd->specialisationState().setFloatConstant(3, 1.f);
  cmd->specialisationState().setFloatConstant(4, 0.2f);
  cmd->draw(3);

  cmd->endRenderPass(); // End our render pass
  cmd->end();           // Finish recording

  // This semaphore will be signaled when rendering has finished and the frame buffer is ready to be
  // presented on our window.
  auto renderFinishedSemaphore = device->createSemaphore("RenderFinished");

  // This fence will be signaled when the frame buffer has been blitted to the swapchain image and
  // we are ready to start the next frame.
  auto frameFinishedFence = device->createFence("FrameFinished");

  // Now we open our window.
  window->open();

  // And start our main application loop.
  while (!window->shouldClose()) {

    // This will trigger re-creations of the swapchain and make sure that window->shouldClose()
    // actually returns true when the user closed the window.
    window->update();

    // Wait until the last frame has been fully processed. Afterwards, reset the fence so that we
    // can use it once more this frame.
    device->waitForFence(frameFinishedFence);
    device->resetFence(frameFinishedFence);

    // Our command buffer has been recorded already, so we can just submit it. Once it has been
    // processed, the renderFinishedSemaphore will be signaled.
    cmd->submit({}, {}, {renderFinishedSemaphore});

    // Present the color attachment of the render pass on the window. This operation will wait for
    // the renderFinishedSemaphore and signal the frameFinishedFence so that we know when to start
    // the next frame.
    window->present(
        renderPass->getAttachments()[0].mImage, renderFinishedSemaphore, frameFinishedFence);

    // Prevent the GPU from over-heating :)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // The window has been closed. We wait for all pending operations and then all objects will be
  // deleted automatically in the correct order.
  device->waitIdle();

  return 0;
}
