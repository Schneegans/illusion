////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RenderGraph.hpp"

#include "../Core/Logger.hpp"

#include <iostream>
#include <queue>
#include <unordered_set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderGraph::Resource& RenderGraph::Resource::setName(std::string const& name) {
  mName  = name;
  mDirty = true;
  return *this;
}

RenderGraph::Resource& RenderGraph::Resource::setFormat(vk::Format format) {
  mFormat = format;
  mDirty  = true;
  return *this;
}

RenderGraph::Resource& RenderGraph::Resource::setType(Type type) {
  mType  = type;
  mDirty = true;
  return *this;
}

RenderGraph::Resource& RenderGraph::Resource::setSizing(Sizing sizing) {
  mSizing = sizing;
  mDirty  = true;
  return *this;
}

RenderGraph::Resource& RenderGraph::Resource::setExtent(glm::uvec2 const& extent) {
  mExtent = extent;
  mDirty  = true;
  return *this;
}

RenderGraph::Pass& RenderGraph::Pass::setName(std::string const& name) {
  mName  = name;
  mDirty = true;
  return *this;
}

RenderGraph::Pass& RenderGraph::Pass::addInputAttachment(Resource const& resource) {
  return addResource(resource, {ResourceType::eInputAttachment});
}

RenderGraph::Pass& RenderGraph::Pass::addBlendAttachment(Resource const& resource) {
  return addResource(resource, {ResourceType::eBlendAttachment});
}

RenderGraph::Pass& RenderGraph::Pass::addOutputAttachment(
    Resource const& resource, std::optional<vk::ClearValue> clearValue) {
  return addResource(resource, {ResourceType::eOutputAttachment, clearValue});
}

RenderGraph::Pass& RenderGraph::Pass::setOutputWindow(WindowPtr const& window) {
  mOutputWindow = window;
  mDirty        = true;
  return *this;
}

RenderGraph::Pass& RenderGraph::Pass::setRecordCallback(std::function<void()> const& callback) {
  mRecordCallback = callback;
  mDirty          = true;
  return *this;
}

RenderGraph::Pass& RenderGraph::Pass::addResource(
    Resource const& resource, ResourceInfo const& info) {

  if (mResources.find(&resource) != mResources.end()) {
    throw std::runtime_error("Failed to add resource \"" + resource.mName +
                             "\" to render graph pass \"" + mName +
                             "\": Resource has already been added to this pass!");
  }
  mResources[&resource] = info;
  mDirty                = true;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderGraph::Resource& RenderGraph::addResource() {
  mDirty = true;
  mResources.push_back({});
  return mResources.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RenderGraph::Pass& RenderGraph::addPass() {
  mDirty = true;
  mPasses.push_back({});
  return mPasses.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderGraph::record() {

  if (isDirty()) {

    // validate resources, in- and outputs
    try {
      validate();
    } catch (std::runtime_error e) {
      throw std::runtime_error("Render graph validation failed: " + std::string(e.what()));
    }

    clearDirty();
  }

  std::for_each(mPasses.begin(), mPasses.end(), [](auto const& pass) { pass.mRecordCallback(); });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderGraph::isDirty() const {
  if (mDirty) {
    return true;
  }

  for (auto const& resource : mResources) {
    if (resource.mDirty) {
      return true;
    }
  }

  for (auto const& pass : mPasses) {
    if (pass.mDirty) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderGraph::clearDirty() {
  mDirty = false;

  for (auto& resource : mResources) {
    resource.mDirty = false;
  }

  for (auto& pass : mPasses) {
    pass.mDirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderGraph::validate() const {

  // Check whether each resource of each pass was actually created by this render graph
  for (auto const& pass : mPasses) {
    for (auto const& passResource : pass.mResources) {
      bool ours = false;
      for (auto const& graphResource : mResources) {
        if (&graphResource == passResource.first) {
          ours = true;
          break;
        }
      }
      if (!ours) {
        throw std::runtime_error("Resource \"" + passResource.first->mName + "\" of pass \"" +
                                 pass.mName +
                                 "\" does not belong to this render graph. Did you accidentally "
                                 "create a copy of the reference?");
      }
    }
  }

  // Check whether each resource is used in the graph and that each resource's first use is as
  // output attachment
  for (auto const& resource : mResources) {
    bool used = false;
    for (auto& pass : mPasses) {
      auto passResource = pass.mResources.find(&resource);
      if (passResource != pass.mResources.end()) {
        if (passResource->second.type != Pass::ResourceType::eOutputAttachment) {
          throw std::runtime_error(
              "First use of resource \"" + resource.mName + "\" must be output attachment!");
        }
        used = true;
        break;
      }
    }

    if (!used) {
      throw std::runtime_error("Resource \"" + resource.mName + "\" is not used at all!");
    }
  }

  // Check whether we have exactly one pass with an output window
  uint32_t outputWindows = 0;
  for (auto const& pass : mPasses) {
    if (pass.mOutputWindow) {
      ++outputWindows;
    }
  }

  if (outputWindows != 1) {
    throw std::runtime_error("There must be exactly one output window in the graph.");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
