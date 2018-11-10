////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_MATERIAL_HPP
#define ILLUSION_GRAPHICS_MATERIAL_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

#include <unordered_map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Material {

 public:
  struct PipelineCreateInfo {
    Material const*                                  mMaterial;
    RenderPass const*                                mRenderPass;
    uint32_t                                         mSubPass;
    vk::PrimitiveTopology                            mPrimitiveTopology;
    std::vector<vk::VertexInputBindingDescription>   mInputBindings;
    std::vector<vk::VertexInputAttributeDescription> mInputAttributes;

    bool operator==(PipelineCreateInfo const& other) const {
      return mMaterial == other.mMaterial && mRenderPass == other.mRenderPass &&
             mSubPass == other.mSubPass && mPrimitiveTopology == other.mPrimitiveTopology &&
             mInputBindings == other.mInputBindings && mInputAttributes == other.mInputAttributes;
    }
  };

  // -------------------------------------------------------------------------------- public methods
  Material(
    std::shared_ptr<Context> const& context,
    std::vector<std::string> const& shaderFiles,
    uint32_t                        materialCount);
  virtual ~Material();

  std::shared_ptr<PipelineLayout> const& getLayout() const { return mPipelineLayout; }

  void bind(
    vk::CommandBuffer const&                                cmd,
    std::shared_ptr<RenderPass> const&                      renderPass,
    uint32_t                                                subPass,
    vk::PrimitiveTopology                                   primitiveTopology,
    std::vector<vk::VertexInputBindingDescription> const&   inputBindings   = {},
    std::vector<vk::VertexInputAttributeDescription> const& inputAttributes = {}) const;

  // pipeline cache access -------------------------------------------------------------------------
  static std::shared_ptr<vk::Pipeline> getCachedPipeline(PipelineCreateInfo const& info);
  static void                          clearPipelineCache(RenderPass const* renderPass);
  static void                          clearPipelineCache();

 private:
  // ------------------------------------------------------------------------------- private methods
  std::shared_ptr<vk::Pipeline> createPipeline(PipelineCreateInfo const& info) const;

  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Context>        mContext;
  std::shared_ptr<PipelineLayout> mPipelineLayout;

  static std::unordered_map<
    RenderPass const*,
    std::vector<std::pair<Material::PipelineCreateInfo, std::shared_ptr<vk::Pipeline>>>>
    mPipelineCache;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_MATERIAL_HPP
