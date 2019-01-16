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
#include <unordered_map>

namespace Illusion::Graphics {

class RenderGraph {
 public:
  struct Resource {
    enum class Type { eImage };
    enum class Sizing { eAbsolute, eRelative };

    vk::Format mFormat = vk::Format::eR8G8B8A8Unorm;
    Type       mType   = Type::eImage;
    Sizing     mSizing = Sizing::eRelative;
    glm::vec2  mExtent = glm::vec2(1.f, 1.f);
  };

  struct Pass {

    struct Input {};

    struct Output {
      vk::AttachmentLoadOp mLoadOp = vk::AttachmentLoadOp::eDontCare;
    };

    std::unordered_map<std::string, Input>  mInputs;
    std::unordered_map<std::string, Output> mOutputs;
    std::function<void()>                   mRecordCallback;
  };

  std::unordered_map<std::string, Resource> mResources;
  std::unordered_map<std::string, Pass>     mPasses;

  void render();

 private:
  void validate() const;
  bool isValidResource(std::string const& name) const;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
