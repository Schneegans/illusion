////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ShaderModule.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "spirv.hpp"

#include <SPIRV/GLSL.std.450.h>
#include <spirv_glsl.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Parts of this code is based on Vulkan-EZ                                                       //
// (MIT, Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.)                    //
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Illusion::Graphics {

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

// Map spirv-types to Illusion's PipelineResource::BaseTypes.
static std::unordered_map<spirv_cross::SPIRType::BaseType, PipelineResource::BaseType>
    spirvTypeToBaseType = {
        {spirv_cross::SPIRType::Boolean, PipelineResource::BaseType::eBool},
        {spirv_cross::SPIRType::Char, PipelineResource::BaseType::eChar},
        {spirv_cross::SPIRType::Int, PipelineResource::BaseType::eInt},
        {spirv_cross::SPIRType::UInt, PipelineResource::BaseType::eUint},
        {spirv_cross::SPIRType::Half, PipelineResource::BaseType::eHalf},
        {spirv_cross::SPIRType::Float, PipelineResource::BaseType::eFloat},
        {spirv_cross::SPIRType::Double, PipelineResource::BaseType::eDouble},
        {spirv_cross::SPIRType::Struct, PipelineResource::BaseType::eStruct},
};

////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<PipelineResource::Member> parseMembers(
    spirv_cross::CompilerGLSL& compiler, const spirv_cross::SPIRType& spirType) {

  std::vector<PipelineResource::Member> members;

  for (size_t i(0); i < spirType.member_types.size(); ++i) {
    // Validate member is of a supported type.
    const auto& memberType = compiler.get_type(spirType.member_types[i]);
    if (spirvTypeToBaseType.find(memberType.basetype) == spirvTypeToBaseType.end())
      continue;

    // Create a new PipelineResource::Member entry.
    PipelineResource::Member member;
    member.mBaseType = spirvTypeToBaseType.at(memberType.basetype);
    member.mOffset   = compiler.type_struct_member_offset(spirType, static_cast<uint32_t>(i));
    member.mSize     = compiler.get_declared_struct_member_size(spirType, static_cast<uint32_t>(i));
    member.mVecSize  = memberType.vecsize;
    member.mColumns  = memberType.columns;
    member.mArraySize = (memberType.array.size() == 0) ? 1 : memberType.array[0];
    member.mName      = compiler.get_member_name(spirType.self, static_cast<uint32_t>(i));

    // Recursively process members that are structs.
    if (memberType.basetype == spirv_cross::SPIRType::Struct) {
      member.mMembers = parseMembers(compiler, memberType);
    }

    members.push_back(member);
  }

  // Return the first member info created.
  return members;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class CustomCompiler : public spirv_cross::CompilerGLSL {
 public:
  CustomCompiler(const std::vector<uint32_t>& spirv)
      : spirv_cross::CompilerGLSL(spirv) {
  }

  vk::AccessFlags get_access_flags(const spirv_cross::SPIRType& type) {
    // SPIRV-Cross hack to get the correct readonly and writeonly attributes on ssbos.
    // This isn't working correctly via Compiler::get_decoration(id, spv::DecorationNonReadable) for
    // example. So the code below is extracted from private methods within spirv_cross.cpp. The
    // spirv_cross executable correctly outputs the attributes when converting spirv back to GLSL,
    // but it's own reflection code does not :-(
    auto all_members_flag_mask = spirv_cross::Bitset(~0ULL);
    for (size_t i(0); i < type.member_types.size(); ++i)
      all_members_flag_mask.merge_and(
          get_member_decoration_bitset(type.self, static_cast<uint32_t>(i)));

    auto base_flags = ir.meta[type.self].decoration.decoration_flags;
    base_flags.merge_or(spirv_cross::Bitset(all_members_flag_mask));

    vk::AccessFlags access = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    if (base_flags.get(spv::DecorationNonReadable))
      access = vk::AccessFlagBits::eShaderWrite;
    else if (base_flags.get(spv::DecorationNonWritable))
      access = vk::AccessFlagBits::eShaderRead;

    return access;
  }
};

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderModule::ShaderModule(std::string const& name, DevicePtr const& device,
    ShaderSourcePtr const& source, vk::ShaderStageFlagBits stage,
    std::set<std::string> const& dynamicBuffers)
    : Core::NamedObject(name)
    , mDevice(device)
    , mStage(stage)
    , mSource(source)
    , mDynamicBuffers(dynamicBuffers) {

  reload();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderModule::~ShaderModule() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderModule::requiresReload() const {
  return mSource->requiresReload();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderModule::resetReloadingRequired() {
  return mSource->resetReloadingRequired();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderModule::reload() {
  // Get spirv code. This comes either from file, from inline code or is compiled on-the-fly from
  // GLSL or HLSL. This depends on the ShaderSource.
  auto spirv = mSource->getSpirv(mStage);

  // Create reflection information.
  createReflection(spirv);

  // And the the actual vk::ShaderModule.
  vk::ShaderModuleCreateInfo info;
  info.codeSize = spirv.size() * 4;
  info.pCode    = spirv.data();
  mHandle       = mDevice->createShaderModule(getName(), info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ShaderModulePtr ShaderModule::getHandle() const {
  return mHandle;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ShaderStageFlagBits ShaderModule::getStage() const {
  return mStage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<PipelineResource> const& ShaderModule::getResources() const {
  return mResources;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderModule::createReflection(std::vector<uint32_t> const& spirv) {
  // Parse SPIRV binary.
  CustomCompiler                     compiler(spirv);
  spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
  opts.enable_420pack_extension           = true;
  compiler.set_common_options(opts);

  // Reflect on all resource bindings.
  auto resources = compiler.get_shader_resources();

  // Extract per stage inputs.
  for (auto& resource : resources.stage_inputs) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::eInput;
    pipelineResource.mAccess       = vk::AccessFlagBits::eShaderRead;
    pipelineResource.mLocation     = compiler.get_decoration(resource.id, spv::DecorationLocation);
    pipelineResource.mVecSize      = spirType.vecsize;
    pipelineResource.mColumns      = spirType.columns;
    pipelineResource.mArraySize    = (spirType.array.size() == 0) ? 1 : spirType.array[0];

    auto it = spirvTypeToBaseType.find(spirType.basetype);
    if (it == spirvTypeToBaseType.end())
      continue;

    pipelineResource.mBaseType = it->second;
    pipelineResource.mName     = resource.name;
    mResources.push_back(pipelineResource);
  }

  // Extract per stage outputs.
  for (auto& resource : resources.stage_outputs) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::eOutput;
    pipelineResource.mAccess       = vk::AccessFlagBits::eShaderWrite;
    pipelineResource.mLocation     = compiler.get_decoration(resource.id, spv::DecorationLocation);
    pipelineResource.mVecSize      = spirType.vecsize;
    pipelineResource.mColumns      = spirType.columns;
    pipelineResource.mArraySize    = (spirType.array.size() == 0) ? 1 : spirType.array[0];

    auto it = spirvTypeToBaseType.find(spirType.basetype);
    if (it == spirvTypeToBaseType.end())
      continue;

    pipelineResource.mBaseType = it->second;
    pipelineResource.mName     = resource.name;
    mResources.push_back(pipelineResource);
  }

  // Extract uniform buffers.
  for (auto& resource : resources.uniform_buffers) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    bool isDynamic = mDynamicBuffers.find(resource.name) != mDynamicBuffers.end();

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = isDynamic
                                         ? PipelineResource::ResourceType::eUniformBufferDynamic
                                         : PipelineResource::ResourceType::eUniformBuffer;
    pipelineResource.mAccess  = vk::AccessFlagBits::eUniformRead;
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mSize      = compiler.get_declared_struct_size(spirType);
    pipelineResource.mName      = resource.name;
    pipelineResource.mMembers   = parseMembers(compiler, spirType);
    mResources.push_back(pipelineResource);
  }

  // Extract storage buffers.
  for (auto& resource : resources.storage_buffers) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    bool isDynamic = mDynamicBuffers.find(resource.name) != mDynamicBuffers.end();

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = isDynamic
                                         ? PipelineResource::ResourceType::eStorageBufferDynamic
                                         : PipelineResource::ResourceType::eStorageBuffer;
    pipelineResource.mAccess  = compiler.get_access_flags(spirType);
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mSize      = compiler.get_declared_struct_size(spirType);
    pipelineResource.mName      = resource.name;
    pipelineResource.mMembers   = parseMembers(compiler, spirType);
    mResources.push_back(pipelineResource);
  }

  // Extract separate samplers.
  for (auto& resource : resources.separate_samplers) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::eSampler;
    pipelineResource.mAccess       = vk::AccessFlagBits::eShaderRead;
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mName      = resource.name;
    mResources.push_back(pipelineResource);
  }

  // Extract sampled images (combined sampler + image or texture buffers).
  for (auto& resource : resources.sampled_images) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = (spirType.image.dim == spv::Dim::DimBuffer)
                                         ? PipelineResource::ResourceType::eUniformTexelBuffer
                                         : PipelineResource::ResourceType::eCombinedImageSampler;
    pipelineResource.mAccess  = vk::AccessFlagBits::eShaderRead;
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mName      = resource.name;
    mResources.push_back(pipelineResource);
  }

  // Extract seperate images ('sampled' in vulkan terminology or no sampler attached).
  for (auto& resource : resources.separate_images) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::eSampledImage;
    pipelineResource.mAccess       = vk::AccessFlagBits::eShaderRead;
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mName      = resource.name;

    // Check if there is already a ResourceType::eSampler for this binding point and set. In this
    // case, we can actually merge both into a ResourceType::eCombinedImageSampler.
    bool mergedToCombinedImageSampler = false;
    for (auto& r : mResources) {
      if (r.mSet == pipelineResource.mSet && r.mBinding == pipelineResource.mBinding &&
          r.mResourceType == PipelineResource::ResourceType::eSampler) {
        r.mResourceType              = PipelineResource::ResourceType::eCombinedImageSampler;
        r.mName                      = pipelineResource.mName;
        mergedToCombinedImageSampler = true;
        break;
      }
    }

    if (!mergedToCombinedImageSampler) {
      mResources.push_back(pipelineResource);
    }
  }

  // Extract storage images.
  for (auto& resource : resources.storage_images) {
    auto            nonReadable  = compiler.get_decoration(resource.id, spv::DecorationNonReadable);
    auto            nonWriteable = compiler.get_decoration(resource.id, spv::DecorationNonWritable);
    vk::AccessFlags access = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    if (nonReadable)
      access = vk::AccessFlagBits::eShaderWrite;
    else if (nonWriteable)
      access = vk::AccessFlagBits::eShaderRead;

    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = (spirType.image.dim == spv::Dim::DimBuffer)
                                         ? PipelineResource::ResourceType::eStorageTexelBuffer
                                         : PipelineResource::ResourceType::eStorageImage;
    pipelineResource.mAccess  = access;
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mName      = resource.name;
    mResources.push_back(pipelineResource);
  }

  // Extract subpass inputs.
  for (auto& resource : resources.subpass_inputs) {
    PipelineResource pipelineResource;
    pipelineResource.mResourceType = PipelineResource::ResourceType::eInputAttachment;
    pipelineResource.mStages       = vk::ShaderStageFlagBits::eFragment;
    pipelineResource.mAccess       = vk::AccessFlagBits::eShaderRead;
    pipelineResource.mInputAttachmentIndex =
        compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = 1;
    pipelineResource.mName      = resource.name;
    mResources.push_back(pipelineResource);
  }

  // Extract push constants.
  for (auto& resource : resources.push_constant_buffers) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    // Get the start offset of the push constant buffer since this will differ between shader
    // stages.
    uint32_t offset = ~0;
    for (size_t i(0); i < spirType.member_types.size(); ++i) {
      auto memberType = compiler.get_type(spirType.member_types[i]);
      offset          = std::min(offset, compiler.get_member_decoration(spirType.self,
                                    static_cast<uint32_t>(i), spv::DecorationOffset));
    }

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::ePushConstantBuffer;
    pipelineResource.mAccess       = vk::AccessFlagBits::eShaderRead;
    pipelineResource.mOffset       = offset;
    pipelineResource.mSize         = compiler.get_declared_struct_size(spirType);
    pipelineResource.mName         = resource.name;
    pipelineResource.mMembers      = parseMembers(compiler, spirType);
    mResources.push_back(pipelineResource);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
