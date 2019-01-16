////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RenderGraph.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderGraph::render() {

  // validate resources, in- and outputs
  try {
    validate();
  } catch (std::runtime_error e) {
    throw std::runtime_error("Render graph validation failed: " + std::string(e.what()));
  }

  // create a sequential list of passes
  std::vector<Pass> passes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderGraph::validate() const {

  // validate resources
  for (auto const& resource : mResources) {
    if (resource.first == "SWAPCHAIN") {
      throw std::runtime_error("Resource name \"SWAPCHAIN\" is a reserved resource name!");
    }
  }

  // validate inputs and outputs
  for (auto const& pass : mPasses) {

    for (auto const& input : pass.second.mInputs) {
      // check whether input resource actually exists
      if (!isValidResource(input.first)) {
        throw std::runtime_error("Input \"" + input.first + "\" of pass \"" + pass.first +
                                 "\" is not a valid resource!");
      }

      // check whether pass has not the same in- and output
      if (pass.second.mOutputs.find(input.first) != pass.second.mOutputs.end()) {
        throw std::runtime_error("Pass \"" + pass.first + "\" has resource \"" + input.first +
                                 "\" both, as input and as output. This is not allowed!");
      }
    }

    for (auto const& output : pass.second.mOutputs) {
      // check whether output resource actually exists
      if (!isValidResource(output.first)) {
        throw std::runtime_error("Output \"" + output.first + "\" of pass \"" + pass.first +
                                 "\" is not a valid resource!");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderGraph::isValidResource(std::string const& name) const {
  if (name == "SWAPCHAIN") {
    return true;
  }

  if (mResources.find(name) != mResources.end()) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
