////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_GRAPHICS_STATE_HPP
#define ILLUSION_GRAPHICS_GRAPHICS_STATE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

#include "../Core/BitHash.hpp"

#include <glm/glm.hpp>
#include <optional>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class GraphicsState {

  struct VertexInputState {

    struct Bindings {
      uint32_t            mBinding   = 0;
      uint32_t            mStride    = 0;
      vk::VertexInputRate mInputRate = vk::VertexInputRate::eVertex;
    };

    struct Attributes {
      uint32_t   mLocation = 0;
      uint32_t   mBinding  = 0;
      vk::Format mFormat   = vk::Format::eUndefined;
      uint32_t   mOffset   = 0;
    };

    std::vector<Bindings>   mBindings;
    std::vector<Attributes> mAttributes;
  };

  struct InputAssemblyState {
    vk::PrimitiveTopology mTopology               = vk::PrimitiveTopology::eTriangleStrip;
    bool                  mPrimitiveRestartEnable = false;
  };

  struct TessellationState {
    uint32_t mPatchControlPoints = 0;
  };

  struct ViewportState {
    struct Viewport {
      glm::ivec2 mOffset   = glm::ivec2(0);
      glm::ivec2 mExtend   = glm::ivec2(0);
      float      mMinDepth = 0;
      float      mMaxDepth = 0;
    };

    struct Scissor {
      glm::ivec2 mOffset = glm::ivec2(0);
      glm::ivec2 mExtend = glm::ivec2(0);
    };

    std::vector<Viewport> mViewports;
    std::vector<Scissor>  mScissors;
  };

  struct RasterizationState {
    bool              mDepthClampEnable        = false;
    bool              mRasterizerDiscardEnable = true;
    vk::PolygonMode   mPolygonMode             = vk::PolygonMode::eFill;
    vk::CullModeFlags mCullMode                = vk::CullModeFlagBits::eBack;
    vk::FrontFace     mFrontFace               = vk::FrontFace::eCounterClockwise;
    bool              mDepthBiasEnable         = false;
    float             mDepthBiasConstantFactor = 0.f;
    float             mDepthBiasClamp          = 0.f;
    float             mDepthBiasSlopeFactor    = 0.f;
    float             mLineWidth               = 1.f;
  };

  struct MultisampleState {
    vk::SampleCountFlagBits mRasterizationSamples = vk::SampleCountFlagBits::e1;
    bool                    mSampleShadingEnable  = false;
    float                   mMinSampleShading     = 0.f;
    std::vector<uint32_t>   mSampleMask;
    bool                    mAlphaToCoverageEnable = false;
    bool                    mAlphaToOneEnable      = false;
  };

  struct DepthStencilState {

    struct StencilOpState {
      vk::StencilOp mFailOp      = vk::StencilOp::eZero;
      vk::StencilOp mPassOp      = vk::StencilOp::eKeep;
      vk::StencilOp mDepthFailOp = vk::StencilOp::eZero;
      vk::CompareOp mCompareOp   = vk::CompareOp::eAlways;
      uint32_t      mCompareMask = 0;
      uint32_t      mWriteMask   = 0;
      uint32_t      mReference   = 0;
    };

    bool           mDepthTestEnable       = true;
    bool           mDepthWriteEnable      = true;
    vk::CompareOp  mDepthCompareOp        = vk::CompareOp::eAlways;
    bool           mDepthBoundsTestEnable = false;
    bool           mStencilTestEnable     = false;
    StencilOpState mFront;
    StencilOpState mBack;
    float          mMinDepthBounds = 0.f;
    float          mMaxDepthBounds = 1.f;
  };

  struct ColorBlendState {
    struct AttachmentState {
      bool                    mBlendEnable         = false;
      vk::BlendFactor         mSrcColorBlendFactor = vk::BlendFactor::eOne;
      vk::BlendFactor         mDstColorBlendFactor = vk::BlendFactor::eOne;
      vk::BlendOp             mColorBlendOp        = vk::BlendOp::eAdd;
      vk::BlendFactor         mSrcAlphaBlendFactor = vk::BlendFactor::eOne;
      vk::BlendFactor         mDstAlphaBlendFactor = vk::BlendFactor::eOne;
      vk::BlendOp             mAlphaBlendOp        = vk::BlendOp::eAdd;
      vk::ColorComponentFlags mColorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    };

    bool                         mLogicOpEnable = false;
    vk::LogicOp                  mLogicOp       = vk::LogicOp::eAnd;
    std::vector<AttachmentState> mAttachments;
    std::array<float, 4>         mBlendConstants;
  };

 public:
  void setPipeline(std::shared_ptr<Pipeline> const& pipeline);
  void setDynamicState(std::vector<vk::DynamicState> const& dynamicState);
  void setVertexInputState(VertexInputState const& vertexInputState);
  void setInputAssemblyState(InputAssemblyState const& inputAssemblyState);
  void setTessellationState(TessellationState const& tessellationState);
  void setViewportState(ViewportState const& viewportState);
  void setRasterizationState(RasterizationState const& rasterizationState);
  void setMultisampleState(MultisampleState const& multisampleState);
  void setDepthStencilState(DepthStencilState const& depthStencilState);
  void setColorBlendState(ColorBlendState const& colorBlendState);

  std::shared_ptr<Pipeline> const&     getPipeline() const;
  std::vector<vk::DynamicState> const& getDynamicState() const;
  VertexInputState const&              getVertexInputState() const;
  InputAssemblyState const&            getInputAssemblyState() const;
  TessellationState const&             getTessellationState() const;
  ViewportState const&                 getViewportState() const;
  RasterizationState const&            getRasterizationState() const;
  MultisampleState const&              getMultisampleState() const;
  DepthStencilState const&             getDepthStencilState() const;
  ColorBlendState const&               getColorBlendState() const;

  Core::BitHash const& getHash() const;

 private:
  std::shared_ptr<Pipeline>     mPipeline;
  std::vector<vk::DynamicState> mDynamicState;
  VertexInputState              mVertexInputState;
  InputAssemblyState            mInputAssemblyState;
  TessellationState             mTessellationState;
  ViewportState                 mViewportState;
  RasterizationState            mRasterizationState;
  MultisampleState              mMultisampleState;
  DepthStencilState             mDepthStencilState;
  ColorBlendState               mColorBlendState;

  mutable bool          mDirty = true;
  mutable Core::BitHash mHash;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GRAPHICS_STATE_HPP
