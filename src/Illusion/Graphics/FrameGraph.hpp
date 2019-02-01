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
// In Illusion, the FrameGraph is used to configure your render passes, framebuffer attachments   //
// and all dependencies between the passes. It automatically create RenderPasses and merges them  //
// into subpasses as often as possible. It actively supports parallel CommandBuffer recording by  //
// using secondary CommandBuffers for each subpass.                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////

class FrameGraph : public Core::StaticCreate<FrameGraph>, public Core::NamedObject {
 public:
  enum class ProcessingFlagBits { eNone = 0, eParallelSubpassRecording = 1 << 0 };

  typedef Core::Flags<ProcessingFlagBits> ProcessingFlags;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////

  class Resource {
   public:
    enum class Sizing { eAbsolute, eRelative };
    enum class Access { eReadOnly, eWriteOnly, eReadWrite, eLoad, eLoadWrite, eLoadReadWrite };

    Resource& setName(std::string const& name);
    Resource& setFormat(vk::Format format);
    Resource& setSizing(Sizing sizing);
    Resource& setExtent(glm::vec2 const& extent);
    Resource& setSamples(vk::SampleCountFlagBits const& samples);

    glm::uvec2 getAbsoluteExtent(glm::uvec2 const& windowExtent) const;
    bool       isDepthResource() const;
    bool       isColorResource() const;

    friend class FrameGraph;

   private:
    std::string             mName    = "Unnamed Resource";
    vk::Format              mFormat  = vk::Format::eR8G8B8A8Unorm;
    Sizing                  mSizing  = Sizing::eRelative;
    glm::vec2               mExtent  = glm::vec2(1.f, 1.f);
    vk::SampleCountFlagBits mSamples = vk::SampleCountFlagBits::e1;

    // This is directly accessed by the FrameGraph.
    bool mDirty = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////

  class Pass {
   public:
    Pass& setName(std::string const& name);

    Pass& assignResource(Resource const& resource, Resource::Access access);
    Pass& assignResource(Resource const& resource, vk::ClearValue const& clear);

    Pass& setProcessCallback(std::function<void(CommandBufferPtr)> const& callback);

    Resource const* getDepthAttachment() const;

    friend class FrameGraph;

   private:
    void assignResource(Resource const& resource, Resource::Access access,
        std::optional<vk::ClearValue> const& clear);

    std::unordered_map<Resource const*, Resource::Access> mResources;
    std::unordered_map<Resource const*, vk::ClearValue>   mClearValues;
    std::function<void(CommandBufferPtr)>                 mProcessCallback;
    std::string                                           mName = "Unnamed Pass";

    // This is directly accessed by the FrameGraph.
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  FrameGraph(std::string const& name, DevicePtr const& device, FrameResourceIndexPtr const& index);

  Resource& createResource();
  Pass&     createPass();

  void setOutput(WindowPtr const& window, Pass const& pass, Resource const& resource);
  void process(ProcessingFlags flags = ProcessingFlagBits::eNone);

 private:
  // -----------------------------------------------------------------------------------------------

  struct RenderPassInfo {
    struct SubpassInfo {
      Pass const*                                              mLogicalPass;
      CommandBufferPtr                                         mSecondaryCommandBuffer;
      std::unordered_set<Pass const*>                          mDependencies;
      std::unordered_map<Resource const*, vk::ImageUsageFlags> mResourceUsage;
    };

    std::vector<SubpassInfo>                            mSubpasses;
    std::vector<Resource const*>                        mAttachments;
    std::unordered_map<Resource const*, vk::ClearValue> mClearValues;
    glm::uvec2                                          mExtent = glm::uvec2(0);
    RenderPassPtr                                       mPhysicalPass;
    std::string                                         mName;
  };

  // -----------------------------------------------------------------------------------------------

  struct PerFrame {
    CommandBufferPtr mPrimaryCommandBuffer;
    vk::SemaphorePtr mRenderFinishedSemaphore;
    vk::FencePtr     mFrameFinishedFence;

    std::unordered_map<Resource const*, BackedImagePtr> mAllAttachments;
    std::list<RenderPassInfo>                           mRenderPasses;
    bool                                                mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  bool isDirty() const;
  void clearDirty();
  void validate() const;

  DevicePtr               mDevice;
  Core::ThreadPool        mThreadPool;
  FrameResource<PerFrame> mPerFrame;

  std::list<Resource> mResources;
  std::list<Pass>     mPasses;

  WindowPtr       mOutputWindow;
  Resource const* mOutputAttachment = nullptr;
  Pass const*     mOutputPass       = nullptr;

  bool mDirty = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
