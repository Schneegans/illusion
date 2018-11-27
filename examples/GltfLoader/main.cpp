////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/DisplayPass.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GltfModel.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

int main(int argc, char* argv[]) {
  auto engine = std::make_shared<Illusion::Graphics::Engine>("Triangle Demo");
  auto device = std::make_shared<Illusion::Graphics::Device>(engine->getPhysicalDevice());
  auto window = std::make_shared<Illusion::Graphics::Window>(engine, device);
  window->open();

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    device,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/Triangle.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/Triangle.frag"}});

  auto model =
    std::make_shared<Illusion::Graphics::GltfModel>(device, "data/models/DamagedHelmet.glb");
  model->printInfo();

  return 0;
}
