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

#include "../Core/BitHash.hpp"
#include "DescriptorSetReflection.hpp"
#include "PipelineResource.hpp"

#include <map>
#include <set>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The PipelineReflection stores information on all PipelineResources used by a piepeline. It can //
// be used to create a corresponding vk::PipelineLayout.                                          //
////////////////////////////////////////////////////////////////////////////////////////////////////

class PipelineReflection {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static PipelineReflectionPtr create(Args&&... args) {
    return std::make_shared<PipelineReflection>(args...);
  };

  // Initially, the PipelineReflection is empty. Resources can be added with addResource()
  PipelineReflection(DevicePtr const& device);
  virtual ~PipelineReflection();

  // Adds a new resource to this PipelineReflection. If the mResourceType is eInput, eOutput or
  // ePushConstantBuffer, the resource will be stored directly in this PipelineReflection. Else it
  // will be added to the corresponding DescriptorSetReflection. The mName of the resource is used
  // as a key for storing the resources. When a resource with the same mName is added that was added
  // before, the mStages of the new resource will be appended to those of the previous resource.
  void addResource(PipelineResource const& resource);

  // Returns a reference to a vector containing the individual
  // DescriptorSetReflections of this PipelineReflection.
  std::vector<DescriptorSetReflectionPtr> const& getDescriptorSetReflections() const;

  // Returns all resources which have been added to this PipelineReflection. The returned map is
  // created on-the-fly, hence this operation is quite costly. If this becomes a bottleneck, storing
  // the resources in an additional map could be considered an improvement.
  std::map<std::string, PipelineResource> getResources() const;

  // Returns only the resources of a given type. The returned map is created on-the-fly, hence this
  // operation is quite costly. If this becomes a bottleneck, storing the resources in additional
  // maps could be considered an improvement.
  std::map<std::string, PipelineResource> getResources(PipelineResource::ResourceType type) const;

  // Creates a vk::PipelineLayout for this reflection. It is created lazily; the first call to
  // this method will cause the allocation.
  vk::PipelineLayoutPtr const& getLayout() const;

  // Prints some reflection information to std::cout for debugging purposes.
  void printInfo() const;

 private:
  DevicePtr                               mDevice;
  std::vector<DescriptorSetReflectionPtr> mDescriptorSetReflections;
  std::map<std::string, PipelineResource> mInputs;
  std::map<std::string, PipelineResource> mOutputs;
  std::map<std::string, PipelineResource> mPushConstantBuffers;
  mutable vk::PipelineLayoutPtr           mLayout;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_REFLECTION_HPP
