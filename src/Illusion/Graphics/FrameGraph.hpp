////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
#define ILLUSION_GRAPHICS_RENDER_GRAPH_HPP

#include "FrameResource.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class FrameGraph : public Core::StaticCreate<FrameGraph> {
 public:
  // -----------------------------------------------------------------------------------------------
  class LogicalResource {
   public:
    enum class Type { eImage, eBuffer };
    enum class Sizing { eAbsolute, eRelative };

    LogicalResource& setName(std::string const& name);
    LogicalResource& setFormat(vk::Format format);
    LogicalResource& setType(Type type);
    LogicalResource& setSizing(Sizing sizing);
    LogicalResource& setExtent(glm::uvec2 const& extent);

    friend class FrameGraph;

   private:
    std::string mName   = "Unnamed Resource";
    vk::Format  mFormat = vk::Format::eR8G8B8A8Unorm;
    Type        mType   = Type::eImage;
    Sizing      mSizing = Sizing::eRelative;
    glm::vec2   mExtent = glm::vec2(1.f, 1.f);

    // These members are read and written by the FrameGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  class LogicalPass {
   public:
    LogicalPass& setName(std::string const& name);

    enum class ResourceUsage { eInputAttachment, eBlendAttachment, eOutputAttachment };

    LogicalPass& addResource(LogicalResource const& resource, ResourceUsage usage,
        std::optional<vk::ClearValue> clear = {});

    LogicalPass& addInputAttachment(LogicalResource const& resource);
    LogicalPass& addOutputAttachment(
        LogicalResource const& resource, std::optional<vk::ClearValue> clear = {});
    LogicalPass& addBlendAttachment(LogicalResource const& resource);

    LogicalPass& setOutputWindow(WindowPtr const& window);
    LogicalPass& setProcessCallback(std::function<void(CommandBufferPtr)> const& callback);

    friend class FrameGraph;

   private:
    struct Info {
      ResourceUsage                 mUsage;
      std::optional<vk::ClearValue> mClear;
    };

    std::unordered_map<LogicalResource const*, Info> mLogicalResources;
    std::string                                      mName = "Unnamed Pass";
    WindowPtr                                        mOutputWindow;
    std::function<void(CommandBufferPtr)>            mProcessCallback;

    // These members are read and written by the FrameGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------
  struct PhysicalResource {
    BackedImagePtr mImage;
  };

  // -----------------------------------------------------------------------------------------------
  struct PhysicalPass {
    RenderPassPtr mRenderPass;
  };

  // -----------------------------------------------------------------------------------------------

  FrameGraph(DevicePtr const& device, FrameResourceIndexPtr const& frameIndex);

  LogicalResource& addResource();
  LogicalPass&     addPass();

  void process();

 private:
  bool isDirty() const;
  void clearDirty();
  void validate() const;

  DevicePtr                  mDevice;
  std::list<LogicalResource> mLogicalResources;
  std::list<LogicalPass>     mLogicalPasses;

  struct PerFrame {
    CommandBufferPtr mPrimaryCommandBuffer;
    vk::SemaphorePtr mRenderFinishedSemaphore;
    vk::FencePtr     mFrameFinishedFence;

    std::unordered_map<LogicalResource const*, PhysicalResource> mPhysicalResources;
    std::unordered_map<LogicalPass const*, PhysicalPass>         mPhysicalPasses;
    bool                                                         mDirty = true;
  };

  FrameResource<PerFrame> mPerFrame;

  bool mDirty = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
