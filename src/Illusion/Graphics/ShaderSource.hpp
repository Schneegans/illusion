////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADER_SOURCE_HPP
#define ILLUSION_GRAPHICS_SHADER_SOURCE_HPP

#include "../Core/File.hpp"
#include "../Core/StaticCreate.hpp"
#include "fwd.hpp"

#include <variant>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A ShaderSource is given to the Shader for each stage. Internally, the Shader class uses the    //
// ShaderSource to construct ShaderModules for each stage.                                        //
// A ShaderSource can be either inline code or a shader file on disc. It can contain GLSL, HLSL   //
// or Spir-V code. GLSL and HLSL will be compiled to Spir-V. This will throw a std::runtime_error //
// when compilation failed for some reason. The getSpirv() methods support shader includes,       //
// however a std::runtime_error will be thrown when an include is not found.                      //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Abstract base class for all shader sources.
class ShaderSource {
 public:
  virtual bool                  isDirty() const                         = 0;
  virtual std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) = 0;
};

// -------------------------------------------------------------------------------------------------

// This (abstract) derived class for file based sources handles the reloading of changed files.
class ShaderFile : public ShaderSource {
 public:
  ShaderFile(std::string const& fileName, bool reloadOnChanges);
  bool isDirty() const override;

 protected:
  Core::File mFile;
  bool       mReloadOnChanges;

  // lazy state
  mutable bool                    mDirty = true;
  mutable std::vector<Core::File> mIncludedFiles;
};

// -------------------------------------------------------------------------------------------------

// This (abstract) derived class is for sources of inline code. The method requiresReload() will
// always returns false.
class ShaderCode : public ShaderSource {
 public:
  ShaderCode(std::string code, std::string name);
  bool isDirty() const override;

 protected:
  std::string mCode;
  std::string mName;

  // lazy state
  mutable bool                    mDirty = true;
  mutable std::vector<Core::File> mIncludedFiles;
};

// -------------------------------------------------------------------------------------------------

class GlslFile : public ShaderFile, public Core::StaticCreate<GlslFile> {
 public:
  GlslFile(std::string const& fileName, bool reloadOnChanges = true);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class GlslCode : public ShaderCode, public Core::StaticCreate<GlslCode> {
 public:
  GlslCode(std::string const& code, std::string const& name);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class HlslFile : public ShaderFile, public Core::StaticCreate<HlslFile> {
 public:
  HlslFile(std::string const& fileName, bool reloadOnChanges = true);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class HlslCode : public ShaderCode, public Core::StaticCreate<HlslCode> {
 public:
  HlslCode(std::string const& code, std::string const& name);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class SpirvFile : public ShaderFile, public Core::StaticCreate<SpirvFile> {
 public:
  SpirvFile(std::string const& fileName, bool reloadOnChanges = true);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class SpirvCode : public ShaderSource, public Core::StaticCreate<SpirvCode> {
 public:
  SpirvCode(std::vector<uint32_t> code);

  bool                  isDirty() const override;
  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;

 private:
  std::vector<uint32_t> mCode;

  // lazy state
  mutable bool mDirty = true;
};

// -------------------------------------------------------------------------------------------------

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_SOURCE_HPP
