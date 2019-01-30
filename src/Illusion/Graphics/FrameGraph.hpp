////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
#define ILLUSION_GRAPHICS_RENDER_GRAPH_HPP

#include "../Core/Flags.hpp"
#include "../Core/NamedObject.hpp"
#include "../Core/ThreadPool.hpp"
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

class FrameGraph : public Core::StaticCreate<FrameGraph>, public Core::NamedObject {
 public:
  enum class ResourceSizing { eAbsolute, eRelative };

  enum class ResourceAccess {
    eReadOnly,
    eWriteOnly,
    eReadWrite,
    eLoad,
    eLoadWrite,
    eLoadReadWrite
  };

  enum class ProcessingFlagBits {
    eNone                        = 0,
    eParallelRenderPassRecording = 1 << 0,
    eParallelSubpassRecording    = 1 << 1
  };

  typedef Core::Flags<ProcessingFlagBits> ProcessingFlags;

  // -----------------------------------------------------------------------------------------------

  class LogicalResource {
   public:
    LogicalResource& setName(std::string const& name);
    LogicalResource& setFormat(vk::Format format);
    LogicalResource& setSizing(ResourceSizing sizing);
    LogicalResource& setExtent(glm::vec2 const& extent);
    LogicalResource& setSamples(vk::SampleCountFlagBits const& samples);

    glm::uvec2 getAbsoluteExtent(glm::uvec2 const& windowExtent) const;
    bool       isDepthResource() const;
    bool       isColorResource() const;

    friend class FrameGraph;

   private:
    std::string             mName    = "Unnamed Resource";
    vk::Format              mFormat  = vk::Format::eR8G8B8A8Unorm;
    ResourceSizing          mSizing  = ResourceSizing::eRelative;
    glm::vec2               mExtent  = glm::vec2(1.f, 1.f);
    vk::SampleCountFlagBits mSamples = vk::SampleCountFlagBits::e1;

    // this is directly accessed by the FrameGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  class LogicalPass {
   public:
    LogicalPass& setName(std::string const& name);

    LogicalPass& assignResource(LogicalResource const& resource, ResourceAccess access);
    LogicalPass& assignResource(LogicalResource const& resource, vk::ClearValue const& clear);

    LogicalPass& setProcessCallback(std::function<void(CommandBufferPtr)> const& callback);

    LogicalResource const* getDepthAttachment() const;

    friend class FrameGraph;

   private:
    void assignResource(LogicalResource const& resource, ResourceAccess access,
        std::optional<vk::ClearValue> const& clear);

    std::unordered_map<LogicalResource const*, ResourceAccess> mLogicalResources;
    std::unordered_map<LogicalResource const*, vk::ClearValue> mClearValues;
    std::function<void(CommandBufferPtr)>                      mProcessCallback;
    std::string                                                mName = "Unnamed Pass";

    // this is directly accessed by the FrameGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  FrameGraph(
      std::string const& name, DevicePtr const& device, FrameResourceIndexPtr const& frameIndex);

  LogicalResource& createResource();
  LogicalPass&     createPass();

  void setOutput(WindowPtr const& window, LogicalPass const& pass, LogicalResource const& resource);
  void process(ProcessingFlags flags = ProcessingFlagBits::eNone);

 private:
  // -----------------------------------------------------------------------------------------------
  struct PhysicalResource {
    BackedImagePtr mImage;
  };

  // -----------------------------------------------------------------------------------------------

  struct PhysicalPass {
    struct Subpass {
      LogicalPass const*                                              mLogicalPass;
      CommandBufferPtr                                                mSecondaryCommandBuffer;
      std::unordered_set<LogicalPass const*>                          mDependencies;
      std::unordered_map<LogicalResource const*, vk::ImageUsageFlags> mResourceUsage;
    };

    std::vector<Subpass>                                       mSubpasses;
    std::vector<LogicalResource const*>                        mAttachments;
    std::unordered_map<LogicalResource const*, vk::ClearValue> mClearValues;
    glm::uvec2                                                 mExtent = glm::uvec2(0);
    RenderPassPtr                                              mRenderPass;
    std::string                                                mName;
  };

  // -----------------------------------------------------------------------------------------------

  struct PerFrame {
    CommandBufferPtr mPrimaryCommandBuffer;
    vk::SemaphorePtr mRenderFinishedSemaphore;
    vk::FencePtr     mFrameFinishedFence;

    std::unordered_map<LogicalResource const*, PhysicalResource> mPhysicalResources;
    std::list<PhysicalPass>                                      mPhysicalPasses;
    bool                                                         mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  bool isDirty() const;
  void clearDirty();
  void validate() const;

  DevicePtr               mDevice;
  WindowPtr               mOutputWindow;
  Core::ThreadPool        mThreadPool;
  FrameResource<PerFrame> mPerFrame;

  std::list<LogicalResource> mLogicalResources;
  LogicalResource const*     mOutputResource = nullptr;

  std::list<LogicalPass> mLogicalPasses;
  LogicalPass const*     mOutputPass = nullptr;

  bool mDirty = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
