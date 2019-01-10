////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ShaderSource.hpp"

#include "../Core/File.hpp"
#include "../Core/Logger.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/OSDependent/osinclude.h>
#include <glslang/Public/ShaderLang.h>

namespace Illusion::Graphics {

namespace {

const std::unordered_map<vk::ShaderStageFlagBits, EShLanguage> shaderStageMapping = {
    {vk::ShaderStageFlagBits::eVertex, EShLangVertex},
    {vk::ShaderStageFlagBits::eTessellationControl, EShLangTessControl},
    {vk::ShaderStageFlagBits::eTessellationEvaluation, EShLangTessEvaluation},
    {vk::ShaderStageFlagBits::eGeometry, EShLangGeometry},
    {vk::ShaderStageFlagBits::eFragment, EShLangFragment},
    {vk::ShaderStageFlagBits::eCompute, EShLangCompute}};

////////////////////////////////////////////////////////////////////////////////////////////////////

class Includer : public DirStackFileIncluder {
 public:
  virtual IncludeResult* includeLocal(
      const char* headerName, const char* includerName, size_t inclusionDepth) override {

    auto result = DirStackFileIncluder::includeLocal(headerName, includerName, inclusionDepth);
    if (!result) {
      throw std::runtime_error(
          "Failed to find shader include \"" + std::string(headerName) + "\"!");
    }

    mIncludedFiles.push_back(result->headerName);

    return result;
  }

  virtual IncludeResult* includeSystem(
      const char* headerName, const char* includerName, size_t inclusionDepth) override {
    throw std::runtime_error("System shader includes are not supported yet!");
  }

  std::vector<Core::File> const& getIncludedFiles() {
    return mIncludedFiles;
  }

 private:
  std::vector<Core::File> mIncludedFiles;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<uint32_t> compile(std::string const& code, std::string const& fileName,
    vk::ShaderStageFlagBits vkStage, EShMessages messages, Includer& includer) {

  glslang::InitializeProcess();

  auto        stage     = shaderStageMapping.at(vkStage);
  const char* codes     = code.c_str();
  const char* fileNames = fileName.c_str();

  glslang::TShader shader(stage);
  shader.setStringsWithLengthsAndNames(&codes, nullptr, &fileNames, 1);
  shader.setEntryPoint("main");
  shader.setSourceEntryPoint("main");
  shader.setPreamble("#extension GL_GOOGLE_include_directive : require");

  // Default built in resource limits.
  auto resourceLimits = glslang::DefaultTBuiltInResource;

  if (!shader.parse(&resourceLimits, 100, false, messages, includer)) {
    throw std::runtime_error(shader.getInfoLog());
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
  std::vector<uint32_t> spirv;
  if (program.getIntermediate(stage)) {
    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger);
  }

  // Shutdown glslang library.
  glslang::FinalizeProcess();

  return spirv;
}

} // namespace

// -------------------------------------------------------------------------------------------------

ShaderFile::ShaderFile(std::string const& fileName, bool reloadOnChanges)
    : mFile(fileName)
    , mReloadOnChanges(reloadOnChanges) {
}

bool ShaderFile::requiresReload() const {
  if (!mReloadOnChanges) {
    return false;
  }

  if (mFile.changedOnDisc()) {
    return true;
  }

  for (auto const& f : mIncludedFiles) {
    if (f.changedOnDisc()) {
      return true;
    }
  }

  return false;
}

void ShaderFile::resetReloadingRequired() {
  mFile.resetChangedOnDisc();

  for (auto& f : mIncludedFiles) {
    f.resetChangedOnDisc();
  }
}

// -------------------------------------------------------------------------------------------------

ShaderCode::ShaderCode(std::string const& code, std::string const& name)
    : mCode(code)
    , mName(name) {
}

bool ShaderCode::requiresReload() const {
  return false;
}

void ShaderCode::resetReloadingRequired() {
}

// -------------------------------------------------------------------------------------------------

GlslFile::GlslFile(std::string const& fileName, bool reloadOnChanges)
    : ShaderFile(fileName, reloadOnChanges) {
}

std::vector<uint32_t> GlslFile::getSpirv(vk::ShaderStageFlagBits stage) {
  auto     code = mFile.getContent<std::string>();
  Includer includer;
  auto     spirv = compile(
      code, mFile.getFileName(), stage, EShMessages(EShMsgSpvRules | EShMsgVulkanRules), includer);

  mIncludedFiles = includer.getIncludedFiles();

  return spirv;
}

// -------------------------------------------------------------------------------------------------

GlslCode::GlslCode(std::string const& code, std::string const& name)
    : ShaderCode(code, name) {
}

std::vector<uint32_t> GlslCode::getSpirv(vk::ShaderStageFlagBits stage) {
  Includer includer;
  return compile(mCode, mName, stage, EShMessages(EShMsgSpvRules | EShMsgVulkanRules), includer);
}

// -------------------------------------------------------------------------------------------------

HlslFile::HlslFile(std::string const& fileName, bool reloadOnChanges)
    : ShaderFile(fileName, reloadOnChanges) {
}

std::vector<uint32_t> HlslFile::getSpirv(vk::ShaderStageFlagBits stage) {
  auto     code = mFile.getContent<std::string>();
  Includer includer;
  auto     spirv = compile(code, mFile.getFileName(), stage,
      EShMessages(EShMsgSpvRules | EShMsgVulkanRules | EShMsgReadHlsl), includer);

  mIncludedFiles = includer.getIncludedFiles();

  return spirv;
}

// -------------------------------------------------------------------------------------------------

HlslCode::HlslCode(std::string const& code, std::string const& name)
    : ShaderCode(code, name) {
}

std::vector<uint32_t> HlslCode::getSpirv(vk::ShaderStageFlagBits stage) {
  Includer includer;
  return compile(mCode, mName, stage,
      EShMessages(EShMsgSpvRules | EShMsgVulkanRules | EShMsgReadHlsl), includer);
}

// -------------------------------------------------------------------------------------------------

SpirvFile::SpirvFile(std::string const& fileName, bool reloadOnChanges)
    : ShaderFile(fileName, reloadOnChanges) {
}

std::vector<uint32_t> SpirvFile::getSpirv(vk::ShaderStageFlagBits stage) {
  return mFile.getContent<std::vector<uint32_t>>();
}

// -------------------------------------------------------------------------------------------------

SpirvCode::SpirvCode(std::vector<uint32_t> const& code)
    : mCode(code) {
}

bool SpirvCode::requiresReload() const {
  return false;
}

std::vector<uint32_t> SpirvCode::getSpirv(vk::ShaderStageFlagBits stage) {
  return mCode;
}

} // namespace Illusion::Graphics
