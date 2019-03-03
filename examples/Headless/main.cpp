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
#include <Illusion/Graphics/Texture.hpp>

#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// In this rather simple example we will render an image and save it to a file. We will do this   //
// in headless-mode - no window or swapchain will be created. We will also have no main loop as   //
// we will only render one frame.                                                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {

  // Enable trace output. This is useful to see Vulkan object lifetime.
  Illusion::Core::Logger::enableTrace = true;

  // These two things nearly every application will need: an instance and a device. We enable
  // eHeadlessMode as we won't create a window and therefore do not need to initialize our windowing
  // toolkit.
  auto instance = Illusion::Graphics::Instance::create(
      "TriangleDemo", Illusion::Graphics::Instance::OptionBits::eHeadlessMode);
  auto device = Illusion::Graphics::Device::create("Device", instance->getPhysicalDevice());

  // Create a shader program. The shader stages a deduced from the file extensions.
  auto shader = Illusion::Graphics::Shader::createFromFiles(
      "TriangleShader", device, {"data/Headless/Triangle.vert", "data/Headless/Triangle.frag"});

  // All rendering is done inside an active render pass. This render pass creates an associated
  // frame buffer with the given attachments (a color buffer in this case). We use a fixed
  // resolution for the render pass.
  auto renderPass = Illusion::Graphics::LazyRenderPass::create("RenderPass", device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(glm::uvec2(800, 600));

  // The color our framebuffer attachment will be cleared to. This is going to be the background
  // color of the final image file.
  vk::ClearColorValue clearColor(std::array<float, 4>{{0.f, 0.f, 0.f, 1.f}});

  // Now create and record the command buffer.
  auto cmd = Illusion::Graphics::CommandBuffer::create("CommandBuffer", device);
  cmd->graphicsState().addViewport({glm::vec2(800, 600)});      // Set the viewport
  cmd->setShader(shader);                                       // Set the shader
  cmd->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse); // Begin recording
  cmd->beginRenderPass(renderPass, {clearColor});               // Begin our render pass
  cmd->draw(3);                                                 // Draw three vertices
  cmd->endRenderPass();                                         // End our render pass
  cmd->end();                                                   // Finish recording

  // Our command buffer has been recorded, so we can just submit it.
  cmd->submit();

  // We could create a semaphore or fence here for more precise synchonization but as we will read
  // the image to host data we will need quite a big barrier anyways.
  device->waitIdle();

  // Now save the attachment of our render pass to an image file.
  Illusion::Graphics::Texture::saveToFile(
      renderPass->getAttachments()[0].mImage, device, "output.tga");

  // Here we wait for all pending operations and then all objects will be deleted automatically in
  // the correct order.
  device->waitIdle();

  return 0;
}
