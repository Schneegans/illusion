////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DESCRIPTOR_SET_REFLECTION_HPP
#define ILLUSION_GRAPHICS_DESCRIPTOR_SET_REFLECTION_HPP

#include "../Core/BitHash.hpp"
#include "../Core/NamedObject.hpp"
#include "../Core/StaticCreate.hpp"
#include "PipelineResource.hpp"

#include <map>
#include <set>
#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The DescriptorSetReflection stores information on one specific set of PipelineResources. It    //
// can be used to create a corresponding vk::DescriptorSetLayout.                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////

class DescriptorSetReflection : public Core::StaticCreate<DescriptorSetReflection>,
                                public Core::NamedObject {
 public:
  // Initially, the DescriptorSetReflection is empty. Resources can be added with addResource(). It
  // is a good idea to give the object a descriptive name.
  DescriptorSetReflection(std::string const& name, DevicePtr const& device, uint32_t set);
  virtual ~DescriptorSetReflection();

  // Adds a new resource to this DescriptorSetReflection. The mName of the resource is used as a key
  // for storing the resources in an internal map. When a resource with the same mName is added that
  // was added before, the mStages of the new resource will be appended to those of the previous
  // resource.
  // Trying to add a resource with mResourceType eInput, eOutput or ePushConstantBuffer will cause
  // an exception.
  // Trying to add a resource with mSet being not equal to this->mSet will also cause an exception.
  void addResource(PipelineResource const& resource);

  // Returns all resources stored in this DescriptorSetReflection.
  std::map<std::string, PipelineResource> const& getResources() const;

  // Returns only the resources of a given type. The returned map is created on-the-fly, hence this
  // operation is quite costly. If this becomes a bottleneck, storing the resources in additional
  // maps could be considered an improvement.
  std::map<std::string, PipelineResource> getResources(PipelineResource::ResourceType type) const;

  // Returns the set number all resources belong to. This has been given to this
  // DescriptorSetReflection in the constructor.
  uint32_t getSet() const;

  // Creates a vk::DescriptorSetLayout for this reflection. It is created lazily; the first call to
  // this method will cause the allocation.
  vk::DescriptorSetLayoutPtr getLayout() const;

  // Prints some reflection information to std::cout for debugging purposes.
  void printInfo() const;

  // Returns a hash which is based on the contained resources and can be used to identify
  // DescriptorSets which are compatible (not necessarily identical since resource names do not
  // matter, for example).
  Core::BitHash const& getHash() const;

 private:
  DevicePtr                               mDevice;
  std::map<std::string, PipelineResource> mResources;
  uint32_t                                mSet;
  mutable vk::DescriptorSetLayoutPtr      mLayout;
  mutable Core::BitHash                   mHash;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_DESCRIPTOR_SET_REFLECTION_HPP
