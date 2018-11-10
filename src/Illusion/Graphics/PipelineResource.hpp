////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_RESOURCE_HPP
#define ILLUSION_GRAPHICS_PIPELINE_RESOURCE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

#include <string>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// parts of this code is based on Vulkan-EZ
// (MIT, Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.)
////////////////////////////////////////////////////////////////////////////////////////////////////

struct PipelineResource {

  enum class BaseType : int { BOOL, CHAR, INT, UINT, UINT64, HALF, FLOAT, DOUBLE, STRUCT, NONE };

  enum class ResourceType : int {
    INPUT,
    OUTPUT,
    SAMPLER,
    COMBINED_IMAGE_SAMPLER,
    SAMPLED_IMAGE,
    STORAGE_IMAGE,
    UNIFORM_TEXEL_BUFFER,
    STORAGE_TEXEL_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    INPUT_ATTACHMENT,
    PUSH_CONSTANT_BUFFER,
    NONE
  };

  struct Member {
    BaseType            mBaseType  = BaseType::NONE;
    uint32_t            mOffset    = 0;
    uint32_t            mSize      = 0;
    uint32_t            mVecSize   = 0;
    uint32_t            mColumns   = 0;
    uint32_t            mArraySize = 0;
    std::string         mName;
    std::vector<Member> mMembers;
  };

  vk::ShaderStageFlags mStages;
  vk::AccessFlags      mAccess;
  ResourceType         mResourceType         = ResourceType::NONE;
  BaseType             mBaseType             = BaseType::NONE;
  uint32_t             mSet                  = 0;
  uint32_t             mBinding              = 0;
  uint32_t             mLocation             = 0;
  uint32_t             mInputAttachmentIndex = 0;
  uint32_t             mVecSize              = 0;
  uint32_t             mColumns              = 0;
  uint32_t             mArraySize            = 0;
  uint32_t             mOffset               = 0;
  uint32_t             mSize                 = 0;
  std::string          mName;
  std::vector<Member>  mMembers;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_RESOURCE_HPP
