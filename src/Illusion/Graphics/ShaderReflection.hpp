////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADER_REFLECTION_HPP
#define ILLUSION_GRAPHICS_SHADER_REFLECTION_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Core/BitHash.hpp"
#include "PipelineResource.hpp"
#include "SetResources.hpp"

#include <map>
#include <set>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderReflection {
 public:
  ShaderReflection();
  virtual ~ShaderReflection();

  void addResource(PipelineResource const& resource);

  std::map<uint32_t, SetResources> const& getSetResources() const;
  std::map<std::string, PipelineResource> getResources() const;
  std::map<std::string, PipelineResource> getResources(PipelineResource::ResourceType type) const;

  void printInfo() const;

 private:
  std::map<uint32_t, SetResources>        mSetResources;
  std::map<std::string, PipelineResource> mInputs;
  std::map<std::string, PipelineResource> mOutputs;
  std::map<std::string, PipelineResource> mPushConstantBuffers;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_REFLECTION_HPP
