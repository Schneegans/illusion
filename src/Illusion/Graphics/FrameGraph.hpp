////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_FRAME_GRAPH_HPP
#define ILLUSION_GRAPHICS_FRAME_GRAPH_HPP

#include "../Core/Flags.hpp"
#include "../Core/NamedObject.hpp"
#include "../Core/ThreadPool.hpp"
#include "FrameResource.hpp"
#include "RenderPass.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// In Illusion, the FrameGraph is used to configure your render passes, framebuffer attachments   //
// and all dependencies between the passes. It automatically creates RenderPasses and merges them //
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
  // For now, resources are "only" images which can be used as input, color or depth attachment.  //
  // You can specify the size and the format of each resource. The resource itself does not hold  //
  // any Vulkan objects, it is rather a description which is later used to create the physical    //
  // resources. When you change one of the resource's properties, the entire frame graph will be  //
  // reconstructed.                                                                               //
  //////////////////////////////////////////////////////////////////////////////////////////////////

  class Resource {
   public:
    enum class Sizing { eAbsolute, eRelative };

    // The name is used for some debug prints and the Vulkan objects which are created for this
    // resource. The size can be either in absolute pixels or in a fraction of the output window's
    // resolution. Have a look at the private members for the default values.
    Resource& setName(std::string const& name);
    Resource& setFormat(vk::Format format);
    Resource& setSizing(Sizing sizing);
    Resource& setExtent(glm::vec2 const& extent);
    Resource& setSamples(vk::SampleCountFlagBits const& samples);

    // Returns either mExtent (when sizing is set to eAbsolute) or mExtent * windowExtent (when
    // sizing is set to eRelative).
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

    Pass& addColorAttachment(Resource const& resource, AccessFlags const& access,
        std::optional<vk::ClearColorValue> clear = {});
    Pass& addDepthAttachment(Resource const& resource, AccessFlags const& access,
        std::optional<vk::ClearDepthStencilValue> clear = {});

    Pass& setProcessCallback(
        std::function<void(CommandBufferPtr const&, std::vector<BackedImagePtr> const&)> const&
            callback);

    friend class FrameGraph;

   private:
    std::vector<Resource const*>                             mAttachments;
    std::unordered_map<Resource const*, AccessFlags>         mAttachmentAccess;
    std::unordered_map<Resource const*, vk::ImageUsageFlags> mAttachmentUsage;
    std::unordered_map<Resource const*, vk::ClearValue>      mAttachmentClear;

    std::function<void(CommandBufferPtr const&, std::vector<BackedImagePtr> const&)>
                mProcessCallback;
    std::string mName = "Unnamed Pass";

    // This is directly accessed by the FrameGraph.
    bool mDirty = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////

  FrameGraph(
      std::string const& name, DeviceConstPtr const& device, FrameResourceIndexPtr const& index);

  // Adds a new resource to the frame graph. Make sure to capture the returned object by reference,
  // else you will be working with a copy.
  Resource& createResource();

  // Adds a new pass to the frame graph. Make sure to capture the returned object by reference,
  // else you will be working with a copy.
  Pass& createPass();

  // Selects a resource of a pass as the output of the frame graph. The given resource will be
  // blitted to the given window. Make sure that the given pass and resource were actually created
  // with the methods above, alse a std::runtime_error will be thrown.
  void setOutput(WindowPtr const& window, Pass const& pass, Resource const& attachment);

  // This triggers construction and execution of the frame graph. Call this once per frame.
  void process(ProcessingFlags const& flags = ProcessingFlagBits::eNone);

 private:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////

  struct RenderPassInfo {
    struct Subpass : public RenderPass::Subpass {
      Pass const*                     mPass;
      CommandBufferPtr                mSecondaryCommandBuffer;
      std::unordered_set<Pass const*> mDependencies;
    };

    RenderPassPtr mRenderPass;
    glm::uvec2    mExtent = glm::uvec2(0);
    std::string   mName;

    std::vector<Subpass> mSubpasses;

    std::vector<Resource const*>                             mAttachments;
    std::unordered_map<Resource const*, AccessFlags>         mAttachmentAccess;
    std::unordered_map<Resource const*, vk::ImageUsageFlags> mAttachmentUsage;
    std::unordered_map<Resource const*, vk::ClearValue>      mAttachmentClear;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////

  struct PerFrame {
    CommandBufferPtr mPrimaryCommandBuffer;
    vk::SemaphorePtr mRenderFinishedSemaphore;
    vk::FencePtr     mFrameFinishedFence;

    std::unordered_map<Resource const*, BackedImagePtr> mAllAttachments;
    std::list<RenderPassInfo>                           mRenderPasses;
    bool                                                mDirty = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////

  bool isDirty() const;
  void clearDirty();
  void validate() const;

  DeviceConstPtr          mDevice;
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

#endif // ILLUSION_GRAPHICS_FRAME_GRAPH_HPP
