////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ToneMapping.hpp"

#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Shader.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////

ToneMapping::ToneMapping(Illusion::Graphics::DeviceConstPtr const& device)
    : mShader(Illusion::Graphics::Shader::createFromFiles("ToneMappingShader", device,
          {"data/DeferredRendering/shaders/Quad.vert",
              "data/DeferredRendering/shaders/ToneMapping.frag"})) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ToneMapping::draw(Illusion::Graphics::CommandBufferPtr const& cmd,
    std::vector<Illusion::Graphics::BackedImagePtr> const&         inputAttachments) {

  cmd->setShader(mShader);

  for (size_t i(0); i < inputAttachments.size(); ++i) {
    cmd->bindingState().setInputAttachment(inputAttachments[i], 0, i);
  }

  cmd->draw(3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
