////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Logger.hpp>
#include <Illusion/Graphics/RenderGraph.hpp>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  Illusion::Graphics::RenderGraph graph;

  // create resources ------------------------------------------------------------------------------
  graph.mResources["albedo"].mFormat = vk::Format::eR8G8B8A8Unorm;
  graph.mResources["normal"].mFormat = vk::Format::eR8G8B8A8Unorm;
  graph.mResources["depth"].mFormat  = vk::Format::eD32Sfloat;
  graph.mResources["hdr"].mFormat    = vk::Format::eR32G32B32A32Sfloat;
  graph.mResources["ldr"].mFormat    = vk::Format::eR8G8B8A8Unorm;

  // create passes ---------------------------------------------------------------------------------
  graph.mPasses["gbuffer"].mOutputs["albedo"].mLoadOp = vk::AttachmentLoadOp::eClear;
  graph.mPasses["gbuffer"].mOutputs["normal"].mLoadOp = vk::AttachmentLoadOp::eClear;
  graph.mPasses["gbuffer"].mOutputs["depth"].mLoadOp  = vk::AttachmentLoadOp::eClear;

  graph.mPasses["lighting"].mInputs["albedo"];
  graph.mPasses["lighting"].mInputs["normal"];
  graph.mPasses["lighting"].mInputs["depth"];
  graph.mPasses["lighting"].mOutputs["hdr"].mLoadOp = vk::AttachmentLoadOp::eDontCare;

  graph.mPasses["tonemapping"].mInputs["hdr"];
  graph.mPasses["tonemapping"].mOutputs["ldr"].mLoadOp = vk::AttachmentLoadOp::eDontCare;

  graph.mPasses["gui"].mInputs["ldr"];
  graph.mPasses["gui"].mOutputs["SWAPCHAIN"].mLoadOp = vk::AttachmentLoadOp::eDontCare;

  // create record callbacks -----------------------------------------------------------------------
  graph.mPasses["gbuffer"].mRecordCallback = []() {
    ILLUSION_MESSAGE << "Record gbuffer pass!" << std::endl;
  };

  graph.mPasses["lighting"].mRecordCallback = []() {
    ILLUSION_MESSAGE << "Record lighting pass!" << std::endl;
  };

  graph.mPasses["tonemapping"].mRecordCallback = []() {
    ILLUSION_MESSAGE << "Record tonemapping pass!" << std::endl;
  };

  graph.mPasses["gui"].mRecordCallback = []() {
    ILLUSION_MESSAGE << "Record gui pass!" << std::endl;
  };

  // do one rendering step -------------------------------------------------------------------------

  graph.render();

  return 0;
}
