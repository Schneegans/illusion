////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Floor.hpp"

#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Shader.hpp>
#include <Illusion/Graphics/Texture.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////

Floor::Floor(Illusion::Graphics::DeviceConstPtr const& device)
    : mAlbedoTexture(Illusion::Graphics::Texture::createFromFile("FloorAlbedo", device,
          "data/DeferredRendering/textures/albedo.jpg",
          Illusion::Graphics::Device::createSamplerInfo(vk::Filter::eLinear,
              vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat)))
    , mNormalTexture(Illusion::Graphics::Texture::createFromFile("FloorNormal", device,
          "data/DeferredRendering/textures/normal.jpg",
          Illusion::Graphics::Device::createSamplerInfo(vk::Filter::eLinear,
              vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat)))
    , mShader(Illusion::Graphics::Shader::createFromFiles("FloorShader", device,
          {"data/DeferredRendering/shaders/Floor.vert",
              "data/DeferredRendering/shaders/Floor.frag"})) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Floor::update(glm::mat4 const& matVP) {
  mMatVP = matVP;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Floor::draw(Illusion::Graphics::CommandBufferPtr const& cmd) {

  cmd->bindingState().setTexture(mAlbedoTexture, 0, 0);
  cmd->bindingState().setTexture(mNormalTexture, 0, 1);

  // The indices are provided as a triangle strip
  cmd->setShader(mShader);
  cmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleStrip);
  cmd->pushConstants(mMatVP);
  cmd->draw(4);

  cmd->bindingState().reset(0, 0);
  cmd->bindingState().reset(0, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
