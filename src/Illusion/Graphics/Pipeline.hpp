////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_HPP
#define ILLUSION_GRAPHICS_PIPELINE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "DescriptorPool.hpp"

#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Pipeline {

 public:
  // -------------------------------------------------------------------------------- public methods
  Pipeline(
    std::shared_ptr<Context> const&                   context,
    std::vector<std::shared_ptr<ShaderModule>> const& modules);
  virtual ~Pipeline();

  std::shared_ptr<vk::PipelineLayout> const&        getLayout() const { return mLayout; }
  std::shared_ptr<ShaderReflection> const&          getReflection() const { return mReflection; }
  std::vector<std::shared_ptr<ShaderModule>> const& getModules() const { return mModules; }

  // descriptor sets -------------------------------------------------------------------------------

  // void useDescriptorSet(
  //   vk::CommandBuffer const& cmd, vk::DescriptorSet const& descriptorSet, uint32_t set = 0)
  //   const;

  std::shared_ptr<vk::DescriptorSet> allocateDescriptorSet(uint32_t setNum = 0);

  // push constants --------------------------------------------------------------------------------

  // void setPushConstant(
  //   vk::CommandBuffer const& cmd,
  //   vk::ShaderStageFlags     stages,
  //   uint32_t                 size,
  //   uint8_t*                 data,
  //   uint32_t                 offset = 0) const;

  // template <typename T>
  // void setPushConstant(
  //   vk::CommandBuffer const& cmd, vk::ShaderStageFlags stages, T data, uint32_t offset = 0) const
  //   {

  //   setPushConstant(cmd, stages, sizeof(T), reinterpret_cast<uint8_t*>(&data), offset);
  // }

  // template <typename T>
  // void setPushConstant(vk::CommandBuffer const& cmd, T data) const {
  //   setPushConstant(cmd, T::getActiveStages(), sizeof(T), reinterpret_cast<uint8_t*>(&data));
  // }

 private:
  // ------------------------------------------------------------------------------- private methods
  void createReflection();
  void createDescriptorPools();
  void createLayout();

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Context>                     mContext;
  std::vector<std::shared_ptr<ShaderModule>>   mModules;
  std::shared_ptr<ShaderReflection>            mReflection;
  std::vector<std::shared_ptr<DescriptorPool>> mDescriptorPools;
  std::shared_ptr<vk::PipelineLayout>          mLayout;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_HPP
