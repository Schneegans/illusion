////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_GRAPHICS_STATE_HPP
#define ILLUSION_GRAPHICS_GRAPHICS_STATE_HPP

#include "fwd.hpp"

#include "../Core/BitHash.hpp"

#include <glm/glm.hpp>
#include <map>
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This GraphicsState is used as a member of each CommandBuffer. Based on the stored information, //
// a vk::Pipeline will be created by the CommandBuffer. The method getHash() can be used to cache //
// vk::Pipelines.                                                                                 //
// The default state of each property can be seen in the private part at the end of this file.    //
////////////////////////////////////////////////////////////////////////////////////////////////////

class GraphicsState {
 public:
  // Inner types -----------------------------------------------------------------------------------

  struct BlendAttachment {
    bool                    mBlendEnable         = true;
    vk::BlendFactor         mSrcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    vk::BlendFactor         mDstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    vk::BlendOp             mColorBlendOp        = vk::BlendOp::eAdd;
    vk::BlendFactor         mSrcAlphaBlendFactor = vk::BlendFactor::eOne;
    vk::BlendFactor         mDstAlphaBlendFactor = vk::BlendFactor::eZero;
    vk::BlendOp             mAlphaBlendOp        = vk::BlendOp::eAdd;
    vk::ColorComponentFlags mColorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    bool operator==(BlendAttachment const& other) const;
  };

  struct Viewport {
    glm::vec2 mExtend   = glm::vec2(0);
    glm::vec2 mOffset   = glm::vec2(0);
    float     mMinDepth = 0;
    float     mMaxDepth = 1;

    bool operator==(Viewport const& other) const;
  };

  struct Scissor {
    glm::uvec2 mExtend = glm::uvec2(0);
    glm::ivec2 mOffset = glm::ivec2(0);

    bool operator==(Scissor const& other) const;
  };

  // -----------------------------------------------------------------------------------------------

  GraphicsState(DevicePtr device);

  void reset();

  // clang-format off

  // Color Blend State -----------------------------------------------------------------------------
  void                                setBlendLogicOpEnable(bool val);
  bool                                getBlendLogicOpEnable() const;
  void                                setBlendLogicOp(vk::LogicOp val);
  vk::LogicOp                         getBlendLogicOp() const;
  void                                setBlendConstants(std::array<float, 4> const& val);
  std::array<float, 4> const&         getBlendConstants() const;

  // If no blend attachments are defined, the pipeline will use on default-constructed
  // BlendAttachment for each color attachment of the current renderpass. 
  void                                addBlendAttachment(BlendAttachment const& val);
  void                                setBlendAttachments(std::vector<BlendAttachment> const& val);
  std::vector<BlendAttachment> const& getBlendAttachments() const;

  // Depth Stencil State ---------------------------------------------------------------------------
  void                                setDepthTestEnable(bool val);
  bool                                getDepthTestEnable() const;
  void                                setDepthWriteEnable(bool val);
  bool                                getDepthWriteEnable() const;
  void                                setDepthCompareOp(vk::CompareOp val);
  vk::CompareOp                       getDepthCompareOp() const;
  void                                setDepthBoundsTestEnable(bool val);
  bool                                getDepthBoundsTestEnable() const;
  void                                setStencilTestEnable(bool val);
  bool                                getStencilTestEnable() const;
  void                                setStencilFrontFailOp(vk::StencilOp val);
  vk::StencilOp                       getStencilFrontFailOp() const;
  void                                setStencilFrontPassOp(vk::StencilOp val);
  vk::StencilOp                       getStencilFrontPassOp() const;
  void                                setStencilFrontDepthFailOp(vk::StencilOp val);
  vk::StencilOp                       getStencilFrontDepthFailOp() const;
  void                                setStencilFrontCompareOp(vk::CompareOp val);
  vk::CompareOp                       getStencilFrontCompareOp() const;
  void                                setStencilFrontCompareMask(uint32_t val);
  uint32_t                            getStencilFrontCompareMask() const;
  void                                setStencilFrontWriteMask(uint32_t val);
  uint32_t                            getStencilFrontWriteMask() const;
  void                                setStencilFrontReference(uint32_t val);
  uint32_t                            getStencilFrontReference() const;
  void                                setStencilBackFailOp(vk::StencilOp val);
  vk::StencilOp                       getStencilBackFailOp() const;
  void                                setStencilBackPassOp(vk::StencilOp val);
  vk::StencilOp                       getStencilBackPassOp() const;
  void                                setStencilBackDepthFailOp(vk::StencilOp val);
  vk::StencilOp                       getStencilBackDepthFailOp() const;
  void                                setStencilBackCompareOp(vk::CompareOp val);
  vk::CompareOp                       getStencilBackCompareOp() const;
  void                                setStencilBackCompareMask(uint32_t val);
  uint32_t                            getStencilBackCompareMask() const;
  void                                setStencilBackWriteMask(uint32_t val);
  uint32_t                            getStencilBackWriteMask() const;
  void                                setStencilBackReference(uint32_t val);
  uint32_t                            getStencilBackReference() const;
  void                                setMinDepthBounds(float val);
  float                               getMinDepthBounds() const;
  void                                setMaxDepthBounds(float val);
  float                               getMaxDepthBounds() const;

  // Input Assembly State --------------------------------------------------------------------------
  void                                setTopology(vk::PrimitiveTopology val);
  vk::PrimitiveTopology               getTopology() const;
  void                                setPrimitiveRestartEnable(bool val);
  bool                                getPrimitiveRestartEnable() const;

  // Multisample State -----------------------------------------------------------------------------
  void                                setRasterizationSamples(vk::SampleCountFlagBits val);
  vk::SampleCountFlagBits             getRasterizationSamples() const;
  void                                setSampleShadingEnable(bool val);
  bool                                getSampleShadingEnable() const;
  void                                setMinSampleShading(float val);
  float                               getMinSampleShading() const;
  void                                setAlphaToCoverageEnable(bool val);
  bool                                getAlphaToCoverageEnable() const;
  void                                setAlphaToOneEnable(bool val);
  bool                                getAlphaToOneEnable() const;
  void                                setSampleMask(const std::vector<uint32_t>& val);
  std::vector<uint32_t>               getSampleMask() const;

  // Rasterization State ---------------------------------------------------------------------------
  void                                setDepthClampEnable(bool val);
  bool                                getDepthClampEnable() const;
  void                                setRasterizerDiscardEnable(bool val);
  bool                                getRasterizerDiscardEnable() const;
  void                                setPolygonMode(vk::PolygonMode val);
  vk::PolygonMode                     getPolygonMode() const;
  void                                setCullMode(const vk::CullModeFlags& val);
  vk::CullModeFlags                   getCullMode() const;
  void                                setFrontFace(vk::FrontFace val);
  vk::FrontFace                       getFrontFace() const;
  void                                setDepthBiasEnable(bool val);
  bool                                getDepthBiasEnable() const;
  void                                setDepthBiasConstantFactor(float val);
  float                               getDepthBiasConstantFactor() const;
  void                                setDepthBiasClamp(float val);
  float                               getDepthBiasClamp() const;
  void                                setDepthBiasSlopeFactor(float val);
  float                               getDepthBiasSlopeFactor() const;
  void                                setLineWidth(float val);
  float                               getLineWidth() const;

  // Tesselation State -----------------------------------------------------------------------------
  void                                setTessellationPatchControlPoints(uint32_t val);
  uint32_t                            getTessellationPatchControlPoints() const;

  // Vertex Input State ----------------------------------------------------------------------------
  void                                addVertexInputBinding(vk::VertexInputBindingDescription const& val);
  void                                setVertexInputBindings(std::vector<vk::VertexInputBindingDescription> const& val);
  std::vector<vk::VertexInputBindingDescription> const& getVertexInputBindings() const;

  void                                addVertexInputAttribute(vk::VertexInputAttributeDescription const& val);
  void                                setVertexInputAttributes(std::vector<vk::VertexInputAttributeDescription> const& val);
  std::vector<vk::VertexInputAttributeDescription> const& getVertexInputAttributes() const;

  // Viewport State --------------------------------------------------------------------------------
  void                                addViewport(Viewport const& val);
  void                                setViewports(std::vector<Viewport> const& val);
  std::vector<Viewport> const&        getViewports() const;

  // If no Scissors are defined, there will be automatically as many default Scissors as there are
  // Viewports. Thy will match the Viewports in size and position.
  void                                addScissor(Scissor const& val);
  void                                setScissors(std::vector<Scissor> const& val);
  std::vector<Scissor> const&         getScissors() const;

  // Dynamic State ---------------------------------------------------------------------------------
  void                                addDynamicState(vk::DynamicState val);
  void                                removeDynamicState(vk::DynamicState val);
  void                                setDynamicState(std::set<vk::DynamicState> const& val);
  std::set<vk::DynamicState> const&   getDynamicState() const;

  // clang-format on

  // -----------------------------------------------------------------------------------------------
  Core::BitHash const& getHash() const;

 private:
  DevicePtr mDevice;

  // Color Blend State------------------------------------------------------------------------------
  bool                         mBlendLogicOpEnable = false;
  vk::LogicOp                  mBlendLogicOp       = vk::LogicOp::eAnd;
  std::vector<BlendAttachment> mBlendAttachments;
  std::array<float, 4>         mBlendConstants = {{1.f, 1.f, 1.f, 1.f}};

  // Depth Stencil State ---------------------------------------------------------------------------
  bool          mDepthTestEnable         = true;
  bool          mDepthWriteEnable        = true;
  vk::CompareOp mDepthCompareOp          = vk::CompareOp::eLess;
  bool          mDepthBoundsTestEnable   = false;
  bool          mStencilTestEnable       = false;
  vk::StencilOp mStencilFrontFailOp      = vk::StencilOp::eZero;
  vk::StencilOp mStencilFrontPassOp      = vk::StencilOp::eKeep;
  vk::StencilOp mStencilFrontDepthFailOp = vk::StencilOp::eZero;
  vk::CompareOp mStencilFrontCompareOp   = vk::CompareOp::eAlways;
  uint32_t      mStencilFrontCompareMask = 0;
  uint32_t      mStencilFrontWriteMask   = 0;
  uint32_t      mStencilFrontReference   = 0;
  vk::StencilOp mStencilBackFailOp       = vk::StencilOp::eZero;
  vk::StencilOp mStencilBackPassOp       = vk::StencilOp::eKeep;
  vk::StencilOp mStencilBackDepthFailOp  = vk::StencilOp::eZero;
  vk::CompareOp mStencilBackCompareOp    = vk::CompareOp::eAlways;
  uint32_t      mStencilBackCompareMask  = 0;
  uint32_t      mStencilBackWriteMask    = 0;
  uint32_t      mStencilBackReference    = 0;
  float         mMinDepthBounds          = 0.f;
  float         mMaxDepthBounds          = 1.f;

  // Input Assembly State --------------------------------------------------------------------------
  vk::PrimitiveTopology mTopology               = vk::PrimitiveTopology::eTriangleStrip;
  bool                  mPrimitiveRestartEnable = false;

  // Multisample State -----------------------------------------------------------------------------
  vk::SampleCountFlagBits mRasterizationSamples = vk::SampleCountFlagBits::e1;
  bool                    mSampleShadingEnable  = false;
  float                   mMinSampleShading     = 0.f;
  std::vector<uint32_t>   mSampleMask;
  bool                    mAlphaToCoverageEnable = false;
  bool                    mAlphaToOneEnable      = false;

  // Rasterization State ---------------------------------------------------------------------------
  bool              mDepthClampEnable        = false;
  bool              mRasterizerDiscardEnable = false;
  vk::PolygonMode   mPolygonMode             = vk::PolygonMode::eFill;
  vk::CullModeFlags mCullMode                = vk::CullModeFlagBits::eBack;
  vk::FrontFace     mFrontFace               = vk::FrontFace::eCounterClockwise;
  bool              mDepthBiasEnable         = false;
  float             mDepthBiasConstantFactor = 0.f;
  float             mDepthBiasClamp          = 0.f;
  float             mDepthBiasSlopeFactor    = 0.f;
  float             mLineWidth               = 1.f;

  // Tesselation State -----------------------------------------------------------------------------
  uint32_t mTessellationPatchControlPoints = 0;

  // Vertex Input State ----------------------------------------------------------------------------
  std::vector<vk::VertexInputBindingDescription>   mVertexInputBindings;
  std::vector<vk::VertexInputAttributeDescription> mVertexInputAttributes;

  // Viewport State --------------------------------------------------------------------------------
  std::vector<Viewport> mViewports;
  std::vector<Scissor>  mScissors;

  // Dynamic State ---------------------------------------------------------------------------------
  std::set<vk::DynamicState> mDynamicState;

  // Dirty State -----------------------------------------------------------------------------------
  mutable bool          mDirty = true;
  mutable Core::BitHash mHash;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GRAPHICS_STATE_HPP
