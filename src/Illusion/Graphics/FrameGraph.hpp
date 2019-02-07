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

  enum class AccessFlagBits { eNone = 0, eRead = 1 << 0, eWrite = 1 << 1, eLoad = 1 << 2 };
  typedef Core::Flags<AccessFlagBits> AccessFlags;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////

  class Resource {
   public:
    enum class Sizing { eAbsolute, eRelative };

    Resource& setName(std::string const& name);
    Resource& setFormat(vk::Format format);
    Resource& setSizing(Sizing sizing);
    Resource& setExtent(glm::vec2 const& extent);
    Resource& setSamples(vk::SampleCountFlagBits const& samples);

    glm::uvec2 getAbsoluteExtent(glm::uvec2 const& windowExtent) const;

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

    Pass& addColorAttachment(Resource const& resource, const AccessFlags& access,
        std::optional<vk::ClearColorValue> clear = {});
    Pass& addDepthAttachment(Resource const& resource, const AccessFlags& access,
        std::optional<vk::ClearDepthStencilValue> clear = {});

    Pass& setProcessCallback(std::function<void(CommandBufferPtr)> const& callback);

    friend class FrameGraph;

   private:
    std::unordered_map<Resource const*, AccessFlags>         mResourceAccess;
    std::unordered_map<Resource const*, vk::ImageUsageFlags> mResourceUsage;
    std::unordered_map<Resource const*, vk::ClearValue>      mResourceClear;

    std::function<void(CommandBufferPtr)> mProcessCallback;
    std::string                           mName = "Unnamed Pass";

    // This is directly accessed by the FrameGraph.
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  FrameGraph(std::string const& name, DevicePtr const& device, FrameResourceIndexPtr const& index);

  Resource& createResource();
  Pass&     createPass();

  void setOutput(WindowPtr const& window, Pass const& pass, Resource const& resource);
  void process(const ProcessingFlags& flags = ProcessingFlagBits::eNone);

 private:
  // -----------------------------------------------------------------------------------------------

  struct RenderPassInfo {
    struct SubpassInfo {
      Pass const*                     mPass;
      CommandBufferPtr                mSecondaryCommandBuffer;
      std::unordered_set<Pass const*> mDependencies;
    };

    RenderPassPtr mRenderPass;
    glm::uvec2    mExtent = glm::uvec2(0);
    std::string   mName;

    std::vector<SubpassInfo>     mSubpasses;
    std::vector<Resource const*> mAttachments;

    std::unordered_map<Resource const*, AccessFlags>         mResourceAccess;
    std::unordered_map<Resource const*, vk::ImageUsageFlags> mResourceUsage;
    std::unordered_map<Resource const*, vk::ClearValue>      mResourceClear;
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
  Resource const* mOutputAttachment             = nullptr;
  Pass const*     mOutputPass                   = nullptr;
  uint32_t        mOutputWindowExtentConnection = 0;

  bool mDirty = true;
};

} // namespace Illusion::Graphics

Illusion::Graphics::FrameGraph::ProcessingFlags operator|(
    Illusion::Graphics::FrameGraph::ProcessingFlagBits bit0,
    Illusion::Graphics::FrameGraph::ProcessingFlagBits bit1);

Illusion::Graphics::FrameGraph::AccessFlags operator|(
    Illusion::Graphics::FrameGraph::AccessFlagBits bit0,
    Illusion::Graphics::FrameGraph::AccessFlagBits bit1);

#endif // ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
