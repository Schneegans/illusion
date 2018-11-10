////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_LAYOUT_HPP
#define ILLUSION_GRAPHICS_PIPELINE_LAYOUT_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class PipelineLayout {

 public:
  // -------------------------------------------------------------------------------- public methods
  PipelineLayout(
    std::shared_ptr<Context> const& context,
    std::vector<std::string> const& shaderFiles,
    uint32_t                        descriptorCount);
  virtual ~PipelineLayout();

  std::shared_ptr<vk::PipelineLayout> const& getLayout() const { return mPipelineLayout; }
  std::shared_ptr<ShaderReflection> const&   getReflection() const { return mProgramReflection; }
  std::vector<std::vector<uint32_t>> const&  getShaderCodes() const { return mShaderCodes; }
  std::vector<std::shared_ptr<ShaderReflection>> const& getStageReflections() const {
    return mStageReflections;
  }

  // descriptor sets -------------------------------------------------------------------------------

  void useDescriptorSet(
    vk::CommandBuffer const& cmd, vk::DescriptorSet const& descriptorSet, int set = 0) const;

  vk::DescriptorSet allocateDescriptorSet(int set = 0) const;
  void              freeDescriptorSet(vk::DescriptorSet const& set) const;

  // push constants --------------------------------------------------------------------------------

  void setPushConstant(
    vk::CommandBuffer const& cmd,
    vk::ShaderStageFlags     stages,
    uint32_t                 size,
    uint8_t*                 data,
    uint32_t                 offset = 0) const;

  template <typename T>
  void setPushConstant(
    vk::CommandBuffer const& cmd, vk::ShaderStageFlags stages, T data, uint32_t offset = 0) const {

    setPushConstant(cmd, stages, sizeof(T), reinterpret_cast<uint8_t*>(&data), offset);
  }

  template <typename T>
  void setPushConstant(vk::CommandBuffer const& cmd, T data) const {
    setPushConstant(cmd, T::getActiveStages(), sizeof(T), reinterpret_cast<uint8_t*>(&data));
  }

 private:
  // ------------------------------------------------------------------------------- private methods
  std::vector<std::vector<uint32_t>>                    loadShaderCodes() const;
  std::vector<std::shared_ptr<ShaderReflection>>        createStageReflections() const;
  std::shared_ptr<ShaderReflection>                     createProgramReflection() const;
  std::shared_ptr<vk::DescriptorPool>                   createDescriptorPool() const;
  std::vector<std::shared_ptr<vk::DescriptorSetLayout>> createDescriptorSetLayouts() const;
  std::shared_ptr<vk::PipelineLayout>                   createPipelineLayout() const;

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Context> mContext;
  std::vector<std::string> mShaderFiles;
  uint32_t                 mDescriptorCount;

  std::vector<std::vector<uint32_t>>                    mShaderCodes;
  std::vector<std::shared_ptr<ShaderReflection>>        mStageReflections;
  std::shared_ptr<ShaderReflection>                     mProgramReflection;
  std::shared_ptr<vk::DescriptorPool>                   mDescriptorPool;
  std::vector<std::shared_ptr<vk::DescriptorSetLayout>> mDescriptorSetLayouts;
  std::shared_ptr<vk::PipelineLayout>                   mPipelineLayout;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_LAYOUT_HPP
