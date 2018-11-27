////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "ShaderModule.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"

#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/OSDependent/osinclude.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

// parts of this code is based on Vulkan-EZ
// (MIT, Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.)

namespace {
enum TOptions {
  EOptionNone                 = 0,
  EOptionIntermediate         = (1 << 0),
  EOptionSuppressInfolog      = (1 << 1),
  EOptionMemoryLeakMode       = (1 << 2),
  EOptionRelaxedErrors        = (1 << 3),
  EOptionGiveWarnings         = (1 << 4),
  EOptionLinkProgram          = (1 << 5),
  EOptionMultiThreaded        = (1 << 6),
  EOptionDumpConfig           = (1 << 7),
  EOptionDumpReflection       = (1 << 8),
  EOptionSuppressWarnings     = (1 << 9),
  EOptionDumpVersions         = (1 << 10),
  EOptionSpv                  = (1 << 11),
  EOptionHumanReadableSpv     = (1 << 12),
  EOptionVulkanRules          = (1 << 13),
  EOptionDefaultDesktop       = (1 << 14),
  EOptionOutputPreprocessed   = (1 << 15),
  EOptionOutputHexadecimal    = (1 << 16),
  EOptionReadHlsl             = (1 << 17),
  EOptionCascadingErrors      = (1 << 18),
  EOptionAutoMapBindings      = (1 << 19),
  EOptionFlattenUniformArrays = (1 << 20),
  EOptionNoStorageFormat      = (1 << 21),
  EOptionKeepUncalled         = (1 << 22),
  EOptionHlslOffsets          = (1 << 23),
  EOptionHlslIoMapping        = (1 << 24),
  EOptionAutoMapLocations     = (1 << 25),
  EOptionDebug                = (1 << 26),
  EOptionStdin                = (1 << 27),
  EOptionOptimizeDisable      = (1 << 28),
  EOptionOptimizeSize         = (1 << 29),
};

EShLanguage MapShaderStage(vk::ShaderStageFlagBits stage) {
  switch (stage) {
  case vk::ShaderStageFlagBits::eVertex:
    return EShLangVertex;

  case vk::ShaderStageFlagBits::eTessellationControl:
    return EShLangTessControl;

  case vk::ShaderStageFlagBits::eTessellationEvaluation:
    return EShLangTessEvaluation;

  case vk::ShaderStageFlagBits::eGeometry:
    return EShLangGeometry;

  case vk::ShaderStageFlagBits::eFragment:
    return EShLangFragment;

  case vk::ShaderStageFlagBits::eCompute:
    return EShLangCompute;

  default:
    return EShLangVertex;
  }
}

void SetMessageOptions(int options, EShMessages& messages) {
  if (options & EOptionRelaxedErrors) messages = (EShMessages)(messages | EShMsgRelaxedErrors);
  if (options & EOptionIntermediate) messages = (EShMessages)(messages | EShMsgAST);
  if (options & EOptionSuppressWarnings)
    messages = (EShMessages)(messages | EShMsgSuppressWarnings);
  if (options & EOptionSpv) messages = (EShMessages)(messages | EShMsgSpvRules);
  if (options & EOptionVulkanRules) messages = (EShMessages)(messages | EShMsgVulkanRules);
  if (options & EOptionOutputPreprocessed)
    messages = (EShMessages)(messages | EShMsgOnlyPreprocessor);
  if (options & EOptionReadHlsl) messages = (EShMessages)(messages | EShMsgReadHlsl);
  if (options & EOptionCascadingErrors) messages = (EShMessages)(messages | EShMsgCascadingErrors);
  if (options & EOptionKeepUncalled) messages = (EShMessages)(messages | EShMsgKeepUncalled);
}

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

static std::vector<PipelineResource::Member> ParseMembers(
  spirv_cross::CompilerGLSL& compiler, const spirv_cross::SPIRType& spirType) {

  std::vector<PipelineResource::Member> members;

  for (auto i = 0U; i < spirType.member_types.size(); ++i) {
    // Validate member is of a supported type.
    const auto& memberType = compiler.get_type(spirType.member_types[i]);
    if (spirvTypeToBaseType.find(memberType.basetype) == spirvTypeToBaseType.end()) continue;

    // Create a new VezMemberInfo entry.
    PipelineResource::Member member;
    member.mBaseType  = spirvTypeToBaseType.at(memberType.basetype);
    member.mOffset    = compiler.type_struct_member_offset(spirType, i);
    member.mSize      = compiler.get_declared_struct_member_size(spirType, i);
    member.mVecSize   = memberType.vecsize;
    member.mColumns   = memberType.columns;
    member.mArraySize = (memberType.array.size() == 0) ? 1 : memberType.array[0];
    member.mName      = compiler.get_member_name(spirType.self, i);

    // Recursively process members that are structs.
    if (memberType.basetype == spirv_cross::SPIRType::Struct) {
      member.mMembers = ParseMembers(compiler, memberType);
    }

    members.push_back(member);
  }

  // Return the first member info created.
  return members;
}

class CustomCompiler : public spirv_cross::CompilerGLSL {
 public:
  CustomCompiler(const std::vector<uint32_t>& spirv)
    : spirv_cross::CompilerGLSL(spirv) {}

  vk::AccessFlags GetAccessFlags(const spirv_cross::SPIRType& type) {
    // SPIRV-Cross hack to get the correct readonly and writeonly attributes on ssbos.
    // This isn't working correctly via Compiler::get_decoration(id, spv::DecorationNonReadable) for
    // example. So the code below is extracted from private methods within spirv_cross.cpp. The
    // spirv_cross executable correctly outputs the attributes when converting spirv back to GLSL,
    // but it's own reflection code does not :-(
    auto all_members_flag_mask = spirv_cross::Bitset(~0ULL);
    for (auto i = 0U; i < type.member_types.size(); ++i)
      all_members_flag_mask.merge_and(get_member_decoration_bitset(type.self, i));

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

std::vector<uint32_t> ShaderModule::compileGlsl(
  std::string const& glsl, vk::ShaderStageFlagBits vkStage) {

  // Get default built in resource limits.
  auto resourceLimits = glslang::DefaultTBuiltInResource;

  glslang::InitializeProcess();

  // Set message options.
  int         options  = EOptionSpv | EOptionVulkanRules | EOptionLinkProgram;
  EShMessages messages = EShMsgDefault;
  SetMessageOptions(options, messages);

  std::vector<uint32_t> spirv;
  std::string           infoLog;

  auto        stage           = MapShaderStage(vkStage);
  const char* shaderText      = glsl.c_str();
  const char* fileNameList[1] = {""};

  glslang::TShader shader(stage);
  shader.setStringsWithLengthsAndNames(&shaderText, nullptr, fileNameList, 1);
  shader.setEntryPoint("main");
  shader.setSourceEntryPoint("main");
  shader.setShiftSamplerBinding(0);
  shader.setShiftTextureBinding(0);
  shader.setShiftImageBinding(0);
  shader.setShiftUboBinding(0);
  shader.setShiftSsboBinding(0);
  shader.setFlattenUniformArrays(false);
  shader.setNoStorageFormat(false);
  if (!shader.parse(&resourceLimits, 100, false, messages)) {
    infoLog = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
    throw std::runtime_error(infoLog);
  }

  // Add shader to new program object.
  glslang::TProgram program;
  program.addShader(&shader);

  // Link program.
  if (!program.link(messages)) {
    infoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
    throw std::runtime_error(infoLog);
  }

  // Map IO for SPIRV generation.
  if (!program.mapIO()) {
    infoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
    throw std::runtime_error(infoLog);
  }

  // Translate to SPIRV.
  if (program.getIntermediate(stage)) {
    std::string         warningsErrors;
    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger);
  }

  // Shutdown glslang library.
  glslang::FinalizeProcess();

  return spirv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderModule::ShaderModule(
  std::shared_ptr<Device> const& device,
  std::vector<uint32_t>&&        spirv,
  vk::ShaderStageFlagBits        stage)
  : mSpirv(spirv)
  , mStage(stage) {
  createReflection();

  vk::ShaderModuleCreateInfo info;
  info.codeSize = mSpirv.size() * 4;
  info.pCode    = mSpirv.data();
  mModule       = device->createShaderModule(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderModule::ShaderModule(
  std::shared_ptr<Device> const& device, std::string const& glsl, vk::ShaderStageFlagBits stage)
  : mSpirv(compileGlsl(glsl, stage))
  , mStage(stage) {

  createReflection();

  vk::ShaderModuleCreateInfo info;
  info.codeSize = mSpirv.size() * 4;
  info.pCode    = mSpirv.data();
  mModule       = device->createShaderModule(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderModule::~ShaderModule() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ShaderStageFlagBits ShaderModule::getStage() const { return mStage; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::ShaderModule> ShaderModule::getModule() const { return mModule; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<PipelineResource> const& ShaderModule::getResources() const { return mResources; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderModule::createReflection() {
  // Parse SPIRV binary.
  CustomCompiler                     compiler(mSpirv);
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
    if (it == spirvTypeToBaseType.end()) continue;

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
    if (it == spirvTypeToBaseType.end()) continue;

    pipelineResource.mBaseType = it->second;
    pipelineResource.mName     = resource.name;
    mResources.push_back(pipelineResource);
  }

  // Extract uniform buffers.
  for (auto& resource : resources.uniform_buffers) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::eUniformBuffer;
    pipelineResource.mAccess       = vk::AccessFlagBits::eUniformRead;
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mSize      = compiler.get_declared_struct_size(spirType);
    pipelineResource.mName      = resource.name;
    pipelineResource.mMembers   = ParseMembers(compiler, spirType);
    mResources.push_back(pipelineResource);
  }

  // Extract storage buffers.
  for (auto& resource : resources.storage_buffers) {
    const auto& spirType = compiler.get_type_from_variable(resource.id);

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::eStorageBuffer;
    pipelineResource.mAccess       = compiler.GetAccessFlags(spirType);
    pipelineResource.mSet     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    pipelineResource.mBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    pipelineResource.mArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
    pipelineResource.mSize      = compiler.get_declared_struct_size(spirType);
    pipelineResource.mName      = resource.name;
    pipelineResource.mMembers   = ParseMembers(compiler, spirType);
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
    mResources.push_back(pipelineResource);
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
    for (auto i = 0U; i < spirType.member_types.size(); ++i) {
      auto memberType = compiler.get_type(spirType.member_types[i]);
      offset =
        std::min(offset, compiler.get_member_decoration(spirType.self, i, spv::DecorationOffset));
    }

    PipelineResource pipelineResource;
    pipelineResource.mStages       = mStage;
    pipelineResource.mResourceType = PipelineResource::ResourceType::ePushConstantBuffer;
    pipelineResource.mAccess       = vk::AccessFlagBits::eShaderRead;
    pipelineResource.mOffset       = offset;
    pipelineResource.mSize         = compiler.get_declared_struct_size(spirType);
    pipelineResource.mName         = resource.name;
    pipelineResource.mMembers      = ParseMembers(compiler, spirType);
    mResources.push_back(pipelineResource);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
