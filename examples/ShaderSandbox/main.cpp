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
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// An example similar to ShaderToy (https://www.shadertoy.com). You can specify a fragment shader //
// on the command line and it will be automatically reloaded when it changes on disc. Use         //
// `ShaderSandbox --help` to see the options.                                                     //
// When compared to the TexturedQuad example, this example is a little more complicated as the    //
// command buffer is re-recorded every frame and push constants are used to upload some data to   //
// the GPU.                                                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  // The fragment shader file to use.
  std::string shaderFile = "data/ShaderSandbox/Sandbox.frag";
  bool        printHelp  = false;

  // The --trace option enables ILLUSION_TRACE output. This mainly shows when Vulkan objects are
  // created and destroyed.
  Illusion::Core::CommandLineOptions args("Renders a full screen texture.");
  args.addOption({"-h", "--help"}, &printHelp, "Print this help");
  args.addOption({"-t", "--trace"}, &Illusion::Core::Logger::enableTrace, "Print trace output");
  args.addOption({"-s", "--shader"}, &shaderFile,
      "The fragment shader file to use. This defaults to data/ShaderSandbox/Sandbox.frag");
  args.parse(argc, argv);

  // When printHelp was set to true, we print a help message and exit.
  if (printHelp) {
    args.printHelp();
    return 0;
  }

  // Then we start setting up our Vulkan resources.
  auto instance = Illusion::Graphics::Instance::create("Shader Sandbox");
  auto device   = Illusion::Graphics::Device::create(instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create(instance, device);

  // Then we load our shader. This shader will be automatically reloaded once it (or any file it
  // includes) changes on disc. To prevent this default behavior, you have to add a fourth "false"
  // parameter to this call. See Illusion/Graphics/Shader.hpp for details.
  auto shader = Illusion::Graphics::Shader::createFromFiles(
      device, {"data/ShaderSandbox/Sandbox.vert", shaderFile});

  // We create a command buffer but do not perform any recording. This will be done each frame.
  auto cmd = Illusion::Graphics::CommandBuffer::create(device);

  // Then we create our render pass with one color attachment.
  auto renderPass = Illusion::Graphics::RenderPass::create(device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);

  // This semaphore will be signaled when rendering has finished and the frame buffer is ready to be
  // presented on our window.
  auto renderFinishedSemaphore = device->createSemaphore();

  // This fence will be signaled when the frame buffer has been blitted to the swapchain image and
  // we are ready to start the next frame.
  auto frameFinishedFence = device->createFence();

  // Use a timer to get the current system time at each frame.
  Illusion::Core::Timer timer;

  // Then we open our window.
  window->open();

  // And start the application loop.
  while (!window->shouldClose()) {

    // This will trigger re-creations of the swapchain and make sure that window->shouldClose()
    // actually returns true when the user closed the window.
    window->update();

    // Wait until the last frame has been fully processed. Afterwards, reset the fence so that we
    // can use it once more this frame.
    device->waitForFences(*frameFinishedFence);
    device->resetFences(*frameFinishedFence);

    // Adapt the render pass and viewport sizes.
    glm::vec2 windowSize = window->pExtent.get();
    renderPass->setExtent(windowSize);
    cmd->graphicsState().setViewports({{windowSize}});

    // Then record our command buffer. This is basically the same as in the TexturedQuad example.
    // One difference is that we have to reset the command buffer before re-recording it. The other
    // is the call to pushConstants to upload the time and the window's aspect ratio to the GPU.
    cmd->reset();
    cmd->begin();
    cmd->setShader(shader);
    cmd->pushConstants(
        std::array<float, 2>{float(timer.getElapsed()), windowSize.x / windowSize.y});
    cmd->beginRenderPass(renderPass);
    cmd->draw(4);
    cmd->endRenderPass();
    cmd->end();

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
