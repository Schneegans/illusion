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

  enum class BaseType {
    BOOL   = 0,
    CHAR   = 1,
    INT    = 2,
    UINT   = 3,
    UINT64 = 4,
    HALF   = 5,
    FLOAT  = 6,
    DOUBLE = 7,
    STRUCT = 8
  };

  enum class ResourceType {
    INPUT                  = 0,
    OUTPUT                 = 1,
    SAMPLER                = 2,
    COMBINED_IMAGE_SAMPLER = 3,
    SAMPLED_IMAGE          = 4,
    STORAGE_IMAGE          = 5,
    UNIFORM_TEXEL_BUFFER   = 6,
    STORAGE_TEXEL_BUFFER   = 7,
    UNIFORM_BUFFER         = 8,
    STORAGE_BUFFER         = 9,
    INPUT_ATTACHMENT       = 10,
    PUSH_CONSTANT_BUFFER   = 11
  };

  struct Member {
    BaseType            mBaseType;
    uint32_t            mOffset;
    uint32_t            mSize;
    uint32_t            mVecSize;
    uint32_t            mColumns;
    uint32_t            mArraySize;
    std::string         mName;
    std::vector<Member> mMembers;
  };

  vk::ShaderStageFlags mStages;
  ResourceType         mResourceType;
  BaseType             mBaseType;
  vk::AccessFlags      mAccess;
  uint32_t             mSet;
  uint32_t             mBinding;
  uint32_t             mLocation;
  uint32_t             mInputAttachmentIndex;
  uint32_t             mVecSize;
  uint32_t             mColumns;
  uint32_t             mArraySize;
  uint32_t             mOffset;
  uint32_t             mSize;
  std::string          mName;
  std::vector<Member>  mMembers;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_RESOURCE_HPP
