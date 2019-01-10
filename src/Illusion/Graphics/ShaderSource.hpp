////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADER_SOURCE_HPP
#define ILLUSION_GRAPHICS_SHADER_SOURCE_HPP

#include "fwd.hpp"

#include "../Core/File.hpp"

#include <variant>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderSource {
 public:
  virtual bool                  requiresReload() const                  = 0;
  virtual void                  resetReloadingRequired()                = 0;
  virtual std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) = 0;
};

// -------------------------------------------------------------------------------------------------

class ShaderFile : public ShaderSource {
 public:
  ShaderFile(std::string const& fileName, bool reloadOnChanges = true);
  bool requiresReload() const override;
  void resetReloadingRequired() override;

 protected:
  Core::File              mFile;
  bool                    mReloadOnChanges;
  std::vector<Core::File> mIncludedFiles;
};

// -------------------------------------------------------------------------------------------------

class ShaderCode : public ShaderSource {
 public:
  ShaderCode(std::string const& code, std::string const& name);
  bool requiresReload() const override;
  void resetReloadingRequired() override;

 protected:
  std::string mCode;
  std::string mName;
};

// -------------------------------------------------------------------------------------------------

class GlslFile : public ShaderFile {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static std::shared_ptr<GlslFile> create(Args&&... args) {
    return std::make_shared<GlslFile>(args...);
  };

  GlslFile(std::string const& fileName, bool reloadOnChanges = true);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class GlslCode : public ShaderCode {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static std::shared_ptr<GlslCode> create(Args&&... args) {
    return std::make_shared<GlslCode>(args...);
  };

  GlslCode(std::string const& code, std::string const& name);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class HlslFile : public ShaderFile {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static std::shared_ptr<HlslFile> create(Args&&... args) {
    return std::make_shared<HlslFile>(args...);
  };

  HlslFile(std::string const& fileName, bool reloadOnChanges = true);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class HlslCode : public ShaderCode {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static std::shared_ptr<HlslCode> create(Args&&... args) {
    return std::make_shared<HlslCode>(args...);
  };

  HlslCode(std::string const& code, std::string const& name);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class SpirvFile : public ShaderFile {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static std::shared_ptr<SpirvFile> create(Args&&... args) {
    return std::make_shared<SpirvFile>(args...);
  };

  SpirvFile(std::string const& fileName, bool reloadOnChanges = true);

  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;
};

// -------------------------------------------------------------------------------------------------

class SpirvCode : public ShaderSource {
 public:
  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static std::shared_ptr<SpirvCode> create(Args&&... args) {
    return std::make_shared<SpirvCode>(args...);
  };

  SpirvCode(std::vector<uint32_t> const& code);

  bool                  requiresReload() const override;
  std::vector<uint32_t> getSpirv(vk::ShaderStageFlagBits stage) override;

 private:
  std::vector<uint32_t> mCode;
};

// -------------------------------------------------------------------------------------------------

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADER_SOURCE_HPP
