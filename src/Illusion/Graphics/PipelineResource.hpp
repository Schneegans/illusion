////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_RESOURCE_HPP
#define ILLUSION_GRAPHICS_PIPELINE_RESOURCE_HPP

#include "fwd.hpp"

#include <string>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// When ShaderModules are loaded, reflection information is stored. This consists of              //
// PipelineResources. So for each Sampler, UniformBuffer, PushConstant, etc. there will be one    //
// PipelineResource you can use to get information from.                                          //
// When the ShaderModules are added to a Shader, the PipelineReflection will be filled with       //
// PipelineResources. You can get an instance of this class from your Shader - this is your main  //
// point to get reflection information from.                                                      //
//                                                                                                //
//  Parts of this code is based on Vulkan-EZ.                                                     //
// (MIT, Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.)                    //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct PipelineResource {

  enum class BaseType : int {
    eBool,
    eChar,
    eInt,
    eUint,
    eUint64,
    eHalf,
    eFloat,
    eDouble,
    eStruct,
    eNone
  };

  enum class ResourceType : int {
    eInput,
    eOutput,
    eSampler,
    eCombinedImageSampler,
    eSampledImage,
    eStorageImage,
    eUniformTexelBuffer,
    eStorageTexelBuffer,
    eUniformBuffer,
    eUniformBufferDynamic,
    eStorageBuffer,
    eStorageBufferDynamic,
    eInputAttachment,
    ePushConstantBuffer,
    eNone
  };

  struct Member {
    BaseType            mBaseType  = BaseType::eNone;
    uint32_t            mOffset    = 0;
    size_t              mSize      = 0;
    uint32_t            mVecSize   = 0;
    uint32_t            mColumns   = 0;
    uint32_t            mArraySize = 0;
    std::string         mName;
    std::vector<Member> mMembers;
  };

  vk::ShaderStageFlags mStages;
  vk::AccessFlags      mAccess;
  ResourceType         mResourceType         = ResourceType::eNone;
  BaseType             mBaseType             = BaseType::eNone;
  uint32_t             mSet                  = 0;
  uint32_t             mBinding              = 0;
  uint32_t             mLocation             = 0;
  uint32_t             mInputAttachmentIndex = 0;
  uint32_t             mVecSize              = 0;
  uint32_t             mColumns              = 0;
  uint32_t             mArraySize            = 0;
  uint32_t             mOffset               = 0;
  size_t               mSize                 = 0;
  std::string          mName;
  std::vector<Member>  mMembers;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_RESOURCE_HPP
