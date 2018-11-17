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
#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class GraphicsState {
 public:
  // -----------------------------------------------------------------------------------------------
  struct ColorBlendState {
    struct AttachmentState {
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
    };

    bool                         mLogicOpEnable = false;
    vk::LogicOp                  mLogicOp       = vk::LogicOp::eAnd;
    std::vector<AttachmentState> mAttachments;
    std::array<float, 4>         mBlendConstants = {{1.f, 1.f, 1.f, 1.f}};
  };

  // -----------------------------------------------------------------------------------------------
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
    vk::CompareOp  mDepthCompareOp        = vk::CompareOp::eLess;
    bool           mDepthBoundsTestEnable = false;
    bool           mStencilTestEnable     = false;
    StencilOpState mFront;
    StencilOpState mBack;
    float          mMinDepthBounds = 0.f;
    float          mMaxDepthBounds = 1.f;
  };

  // -----------------------------------------------------------------------------------------------
  struct InputAssemblyState {
    vk::PrimitiveTopology mTopology               = vk::PrimitiveTopology::eTriangleStrip;
    bool                  mPrimitiveRestartEnable = false;
  };

  // -----------------------------------------------------------------------------------------------
  struct MultisampleState {
    vk::SampleCountFlagBits mRasterizationSamples = vk::SampleCountFlagBits::e1;
    bool                    mSampleShadingEnable  = false;
    float                   mMinSampleShading     = 0.f;
    std::vector<uint32_t>   mSampleMask;
    bool                    mAlphaToCoverageEnable = false;
    bool                    mAlphaToOneEnable      = false;
  };

  // -----------------------------------------------------------------------------------------------
  struct RasterizationState {
    bool              mDepthClampEnable        = false;
    bool              mRasterizerDiscardEnable = false;
    vk::PolygonMode   mPolygonMode             = vk::PolygonMode::eFill;
    vk::CullModeFlags mCullMode                = vk::CullModeFlagBits::eNone;
    vk::FrontFace     mFrontFace               = vk::FrontFace::eCounterClockwise;
    bool              mDepthBiasEnable         = false;
    float             mDepthBiasConstantFactor = 0.f;
    float             mDepthBiasClamp          = 0.f;
    float             mDepthBiasSlopeFactor    = 0.f;
    float             mLineWidth               = 1.f;
  };

  // -----------------------------------------------------------------------------------------------
  struct TessellationState {
    uint32_t mPatchControlPoints = 0;
  };

  // -----------------------------------------------------------------------------------------------
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

  // -----------------------------------------------------------------------------------------------
  struct ViewportState {
    struct Viewport {
      glm::vec2 mOffset   = glm::vec2(0);
      glm::vec2 mExtend   = glm::vec2(0);
      float     mMinDepth = 0;
      float     mMaxDepth = 1;
    };

    struct Scissor {
      glm::ivec2 mOffset = glm::ivec2(0);
      glm::uvec2 mExtend = glm::uvec2(0);
    };

    std::vector<Viewport> mViewports;
    std::vector<Scissor>  mScissors;
  };

  // -----------------------------------------------------------------------------------------------
  void setColorBlendState(ColorBlendState const& colorBlendState);
  void setDepthStencilState(DepthStencilState const& depthStencilState);
  void setDynamicState(std::set<vk::DynamicState> const& dynamicState);
  void setInputAssemblyState(InputAssemblyState const& inputAssemblyState);
  void setMultisampleState(MultisampleState const& multisampleState);
  void setShaderProgram(std::shared_ptr<ShaderProgram> const& shaderProgram);
  void setRasterizationState(RasterizationState const& rasterizationState);
  void setTessellationState(TessellationState const& tessellationState);
  void setVertexInputState(VertexInputState const& vertexInputState);
  void setViewportState(ViewportState const& viewportState);

  // -----------------------------------------------------------------------------------------------
  ColorBlendState const&                getColorBlendState() const;
  DepthStencilState const&              getDepthStencilState() const;
  std::set<vk::DynamicState> const&     getDynamicState() const;
  InputAssemblyState const&             getInputAssemblyState() const;
  MultisampleState const&               getMultisampleState() const;
  std::shared_ptr<ShaderProgram> const& getShaderProgram() const;
  RasterizationState const&             getRasterizationState() const;
  TessellationState const&              getTessellationState() const;
  VertexInputState const&               getVertexInputState() const;
  ViewportState const&                  getViewportState() const;

  // -----------------------------------------------------------------------------------------------
  Core::BitHash const& getHash() const;

 private:
  ColorBlendState                mColorBlendState;
  DepthStencilState              mDepthStencilState;
  std::set<vk::DynamicState>     mDynamicState;
  InputAssemblyState             mInputAssemblyState;
  MultisampleState               mMultisampleState;
  std::shared_ptr<ShaderProgram> mShaderProgram;
  RasterizationState             mRasterizationState;
  TessellationState              mTessellationState;
  VertexInputState               mVertexInputState;
  ViewportState                  mViewportState;

  mutable bool          mDirty = true;
  mutable Core::BitHash mHash;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_GRAPHICS_STATE_HPP
