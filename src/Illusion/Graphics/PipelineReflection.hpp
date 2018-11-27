////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_REFLECTION_HPP
#define ILLUSION_GRAPHICS_PIPELINE_REFLECTION_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Core/BitHash.hpp"
#include "DescriptorSetReflection.hpp"
#include "PipelineResource.hpp"

#include <map>
#include <set>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class PipelineReflection {
 public:
  PipelineReflection(std::shared_ptr<Device> const& device);
  virtual ~PipelineReflection();

  void addResource(PipelineResource const& resource);

  std::map<uint32_t, std::shared_ptr<DescriptorSetReflection>> const& getDescriptorSetReflections()
    const;

  std::map<std::string, PipelineResource> getResources() const;
  std::map<std::string, PipelineResource> getResources(PipelineResource::ResourceType type) const;

  std::shared_ptr<vk::PipelineLayout> const& getLayout() const;
  void                                       printInfo() const;

 private:
  std::shared_ptr<Device>                                      mDevice;
  std::map<uint32_t, std::shared_ptr<DescriptorSetReflection>> mDescriptorSetReflections;
  std::map<std::string, PipelineResource>                      mInputs;
  std::map<std::string, PipelineResource>                      mOutputs;
  std::map<std::string, PipelineResource>                      mPushConstantBuffers;
  mutable std::shared_ptr<vk::PipelineLayout>                  mLayout;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_REFLECTION_HPP
