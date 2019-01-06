////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "spirv.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/OSDependent/osinclude.h>
#include <glslang/Public/ShaderLang.h>

// parts of this code is based on Vulkan-EZ
// (MIT, Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.)

namespace Illusion::Graphics::Spirv {

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

void SetMessageOptions(int options, EShMessages& messages) {
  if (options & EOptionRelaxedErrors)
    messages = (EShMessages)(messages | EShMsgRelaxedErrors);
  if (options & EOptionIntermediate)
    messages = (EShMessages)(messages | EShMsgAST);
  if (options & EOptionSuppressWarnings)
    messages = (EShMessages)(messages | EShMsgSuppressWarnings);
  if (options & EOptionSpv)
    messages = (EShMessages)(messages | EShMsgSpvRules);
  if (options & EOptionVulkanRules)
    messages = (EShMessages)(messages | EShMsgVulkanRules);
  if (options & EOptionOutputPreprocessed)
    messages = (EShMessages)(messages | EShMsgOnlyPreprocessor);
  if (options & EOptionReadHlsl)
    messages = (EShMessages)(messages | EShMsgReadHlsl);
  if (options & EOptionCascadingErrors)
    messages = (EShMessages)(messages | EShMsgCascadingErrors);
  if (options & EOptionKeepUncalled)
    messages = (EShMessages)(messages | EShMsgKeepUncalled);
}

const std::unordered_map<vk::ShaderStageFlagBits, EShLanguage> shaderStageMapping = {
    {vk::ShaderStageFlagBits::eVertex, EShLangVertex},
    {vk::ShaderStageFlagBits::eTessellationControl, EShLangTessControl},
    {vk::ShaderStageFlagBits::eTessellationEvaluation, EShLangTessEvaluation},
    {vk::ShaderStageFlagBits::eGeometry, EShLangGeometry},
    {vk::ShaderStageFlagBits::eFragment, EShLangFragment},
    {vk::ShaderStageFlagBits::eCompute, EShLangCompute}};

} // namespace

std::vector<uint32_t> fromGlsl(std::string const& glsl, vk::ShaderStageFlagBits vkStage) {

  // Get default built in resource limits.
  auto resourceLimits = glslang::DefaultTBuiltInResource;

  glslang::InitializeProcess();

  // Set message options.
  int         options  = EOptionSpv | EOptionVulkanRules | EOptionLinkProgram;
  EShMessages messages = EShMsgDefault;
  SetMessageOptions(options, messages);

  std::vector<uint32_t> spirv;
  std::string           infoLog;

  auto        stage           = shaderStageMapping.at(vkStage);
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
    throw std::runtime_error(program.getInfoLog());
  }

  // Map IO for SPIRV generation.
  if (!program.mapIO()) {
    throw std::runtime_error(program.getInfoLog());
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

} // namespace Illusion::Graphics::Spirv
