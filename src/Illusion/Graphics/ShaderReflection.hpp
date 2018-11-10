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
#include "PipelineResource.hpp"

#include <map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderReflection {
 public:
  ShaderReflection();
  virtual ~ShaderReflection();

  void addResource(PipelineResource const& resource);
  void addResources(std::vector<PipelineResource> const& resources);

  void printInfo() const;

 private:
  std::map<std::string, PipelineResource> mResources;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_REFLECTION_HPP
