////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SET_RESOURCES_HPP
#define ILLUSION_GRAPHICS_SET_RESOURCES_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Core/BitHash.hpp"
#include "PipelineResource.hpp"

#include <map>
#include <set>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SetResources {
 public:
  SetResources(uint32_t set);
  virtual ~SetResources();

  void addResource(PipelineResource const& resource);

  std::map<std::string, PipelineResource> const& getResources() const;
  std::map<std::string, PipelineResource> getResources(PipelineResource::ResourceType type) const;
  uint32_t                                getSet() const;

  Core::BitHash const& getHash() const;

 private:
  std::map<std::string, PipelineResource> mResources;
  uint32_t                                mSet;

  mutable bool          mHashDirty = true;
  mutable Core::BitHash mHash;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SET_RESOURCES_HPP
