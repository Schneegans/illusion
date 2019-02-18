////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/CommandLineOptions.hpp>
#include <Illusion/Core/Logger.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/LazyRenderPass.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// When compared to the Triangle example, this one is only a little bit more involved. Still, no  //
// vertex data is used, however a texture is loaded. Furthermore, the CommandlineOptions class is //
// used to allow the user to specify whether GLSL or HLSL shaders should be used for rendering.   //
// Despite its name, this example actually draws a triangle as well. It is larger thant the       //
// framebuffer inorder to cover the entire window. Like this:                                     //
//                                                                                                //
//  |\                                                                                            //
//  |  \                                                                                          //
//  |____\                                                                                        //
//  |    | \                                                                                      //
//  |    |   \                                                                                    //
//  |____|_____\                                                                                  //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  // Enable trace output. This is useful to see Vulkan object lifetime.
  Illusion::Core::Logger::enableTrace = true;

  // First we parse the command line options. Illusion provides a very basic command line parsing
  // class for this purpose. Here we define two booleans, both set to false per default, which the
  // user may trigger from the command line. For a more comples usage example you can have a look at
  // the ShaderSandbox or the GltfViewer example.
  bool useHLSL   = false;
  bool printHelp = false;

  Illusion::Core::CommandLineOptions args("Renders a full screen texture.");
  args.addOption({"-h", "--help"}, &printHelp, "Print this help");
  args.addOption({"--hlsl"}, &useHLSL, "Use HLSL shaders instead of GLSL shaders");
  args.parse(argc, argv);

  // When printHelp was set to true, we print a help message and exit.
  if (printHelp) {
    args.printHelp();
    return 0;
  }

  // Then we start setting up our Vulkan resources.
  auto instance = Illusion::Graphics::Instance::create("TexturedQuadDemo");
  auto device   = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());
  auto window   = Illusion::Graphics::Window::create("Window", instance, device);

  // Here we load the texture. This supports many file formats (those supported by gli and stb).
  std::string dataDir = "data/TexturedQuad/";
  auto        texture = Illusion::Graphics::Texture::createFromFile(
      "BoxTexture", device, dataDir + "textures/box.dds");

  // Then we load our shader. Based on the command line options, we either load GLSL or HLSL
  // shaders. In theory you could actually mix both - HLSL vertex shader and GLSL fragment shader or
  // vice-versa :)
  auto shader = Illusion::Graphics::Shader::createFromFiles("QuadShader", device,
      useHLSL
          ? std::vector<std::string>{dataDir + "shaders/Quad.vs", dataDir + "shaders/Quad.ps"}
          : std::vector<std::string>{dataDir + "shaders/Quad.vert", dataDir + "shaders/Quad.frag"});

  // Then we create our render pass.
  auto renderPass = Illusion::Graphics::LazyRenderPass::create("RenderPass", device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  // The color our framebuffer attachment will be cleared to.
  vk::ClearColorValue clearColor(std::array<float, 4>{{0.f, 0.f, 0.f, 0.f}});

  // And record our command buffer. The only difference to the Triangle example is that the texture
  // is bound to descriptor set 0 at binding location 0.
  auto cmd = Illusion::Graphics::CommandBuffer::create("CommandBuffer", device);
  cmd->graphicsState().addViewport({glm::vec2(window->pExtent.get())});
  cmd->bindingState().setTexture(texture, 0, 0);
  cmd->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  cmd->setShader(shader);
  cmd->beginRenderPass(renderPass, {clearColor});
  cmd->draw(3);
  cmd->endRenderPass();
  cmd->end();

  // Again we create a fence and a semaphore to synchronize rendering, presentation and our frames.
  auto renderFinishedSemaphore = device->createSemaphore("RenderFinished");
  auto frameFinishedFence      = device->createFence("FrameFinished");

  // Then we open our window.
  window->open();

  // And start the application loop. Exactly the same code as in the Triangle example.
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
