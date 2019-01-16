////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
#define ILLUSION_GRAPHICS_RENDER_GRAPH_HPP

#include "fwd.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <list>
#include <unordered_set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class RenderGraph {
 public:
  // -----------------------------------------------------------------------------------------------
  class Pass;

  class Resource {
   public:
    enum class Type { eImage };
    enum class Sizing { eAbsolute, eRelative };

    Resource& setName(std::string const& name);
    Resource& setFormat(vk::Format format);
    Resource& setType(Type type);
    Resource& setSizing(Sizing sizing);
    Resource& setExtent(glm::uvec2 const& extent);

    friend class RenderGraph;

   private:
    std::string mName   = "Unnamed Resource";
    vk::Format  mFormat = vk::Format::eR8G8B8A8Unorm;
    Type        mType   = Type::eImage;
    Sizing      mSizing = Sizing::eRelative;
    glm::vec2   mExtent = glm::vec2(1.f, 1.f);

    // These members are read and written by the RenderGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  class Pass {
   public:
    Pass& setName(std::string const& name);
    Pass& addInputAttachment(Resource const& resource);
    Pass& addOutputAttachment(
        Resource const& resource, std::optional<vk::ClearValue> clearValue = {});
    Pass& addBlendAttachment(Resource const& resource);
    Pass& setOutputWindow(WindowPtr const& window);
    Pass& setRecordCallback(std::function<void()> const& recordCallback);

    friend class RenderGraph;

   private:
    enum class ResourceType { eInputAttachment, eBlendAttachment, eOutputAttachment };

    struct ResourceInfo {
      ResourceType                  type;
      std::optional<vk::ClearValue> mClearValue;
    };

    Pass& addResource(Resource const& resource, ResourceInfo const& info);

    std::string                                       mName = "Unnamed Pass";
    std::unordered_map<Resource const*, ResourceInfo> mResources;
    WindowPtr                                         mOutputWindow;
    std::function<void()>                             mRecordCallback;

    // These members are read and written by the RenderGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  Resource& addResource();
  Pass&     addPass();

  void record();

 private:
  bool isDirty() const;
  void clearDirty();
  void validate() const;

  std::list<Resource> mResources;
  std::list<Pass>     mPasses;
  bool                mDirty = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
