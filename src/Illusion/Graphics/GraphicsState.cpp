////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GraphicsState.hpp"

#include "PipelineReflection.hpp"
#include "RenderPass.hpp"
#include "ShaderModule.hpp"
#include "ShaderProgram.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsState::GraphicsState(DevicePtr const& device)
  : mDevice(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Color Blend State ------------------------------------------------------------------------------
void GraphicsState::setBlendLogicOpEnable(bool val) {
  mBlendLogicOpEnable = val;
  mDirty              = true;
}
bool GraphicsState::getBlendLogicOpEnable() const { return mBlendLogicOpEnable; }
void GraphicsState::setBlendLogicOp(vk::LogicOp val) {
  mBlendLogicOp = val;
  mDirty        = true;
}
vk::LogicOp GraphicsState::getBlendLogicOp() const { return mBlendLogicOp; }
void        GraphicsState::setBlendConstants(std::array<float, 4> const& val) {
  mBlendConstants = val;
  mDirty          = true;
}
std::array<float, 4> const& GraphicsState::getBlendConstants() const { return mBlendConstants; }
void                        GraphicsState::addBlendAttachment(BlendAttachment const& val) {
  mBlendAttachments.push_back(val);
  mDirty = true;
}
void GraphicsState::setBlendAttachments(std::vector<BlendAttachment> const& val) {
  mBlendAttachments = val;
  mDirty            = true;
}
std::vector<GraphicsState::BlendAttachment> const& GraphicsState::getBlendAttachments() const {
  return mBlendAttachments;
}

// Depth Stencil State ---------------------------------------------------------------------------
void GraphicsState::setDepthTestEnable(bool val) {
  mDepthTestEnable = val;
  mDirty           = true;
}
bool GraphicsState::getDepthTestEnable() const { return mDepthTestEnable; }
void GraphicsState::setDepthWriteEnable(bool val) {
  mDepthWriteEnable = val;
  mDirty            = true;
}
bool GraphicsState::getDepthWriteEnable() const { return mDepthWriteEnable; }
void GraphicsState::setDepthCompareOp(vk::CompareOp val) {
  mDepthCompareOp = val;
  mDirty          = true;
}
vk::CompareOp GraphicsState::getDepthCompareOp() const { return mDepthCompareOp; }
void          GraphicsState::setDepthBoundsTestEnable(bool val) {
  mDepthBoundsTestEnable = val;
  mDirty                 = true;
}
bool GraphicsState::getDepthBoundsTestEnable() const { return mDepthBoundsTestEnable; }
void GraphicsState::setStencilTestEnable(bool val) {
  mStencilTestEnable = val;
  mDirty             = true;
}
bool GraphicsState::getStencilTestEnable() const { return mStencilTestEnable; }
void GraphicsState::setStencilFrontFailOp(vk::StencilOp val) {
  mStencilFrontFailOp = val;
  mDirty              = true;
}
vk::StencilOp GraphicsState::getStencilFrontFailOp() const { return mStencilFrontFailOp; }
void          GraphicsState::setStencilFrontPassOp(vk::StencilOp val) {
  mStencilFrontPassOp = val;
  mDirty              = true;
}
vk::StencilOp GraphicsState::getStencilFrontPassOp() const { return mStencilFrontPassOp; }
void          GraphicsState::setStencilFrontDepthFailOp(vk::StencilOp val) {
  mStencilFrontDepthFailOp = val;
  mDirty                   = true;
}
vk::StencilOp GraphicsState::getStencilFrontDepthFailOp() const { return mStencilFrontDepthFailOp; }
void          GraphicsState::setStencilFrontCompareOp(vk::CompareOp val) {
  mStencilFrontCompareOp = val;
  mDirty                 = true;
}
vk::CompareOp GraphicsState::getStencilFrontCompareOp() const { return mStencilFrontCompareOp; }
void          GraphicsState::setStencilFrontCompareMask(uint32_t val) {
  mStencilFrontCompareMask = val;
  mDirty                   = true;
}
uint32_t GraphicsState::getStencilFrontCompareMask() const { return mStencilFrontCompareMask; }
void     GraphicsState::setStencilFrontWriteMask(uint32_t val) {
  mStencilFrontWriteMask = val;
  mDirty                 = true;
}
uint32_t GraphicsState::getStencilFrontWriteMask() const { return mStencilFrontWriteMask; }
void     GraphicsState::setStencilFrontReference(uint32_t val) {
  mStencilFrontReference = val;
  mDirty                 = true;
}
uint32_t GraphicsState::getStencilFrontReference() const { return mStencilFrontReference; }
void     GraphicsState::setStencilBackFailOp(vk::StencilOp val) {
  mStencilBackFailOp = val;
  mDirty             = true;
}
vk::StencilOp GraphicsState::getStencilBackFailOp() const { return mStencilBackFailOp; }
void          GraphicsState::setStencilBackPassOp(vk::StencilOp val) {
  mStencilBackPassOp = val;
  mDirty             = true;
}
vk::StencilOp GraphicsState::getStencilBackPassOp() const { return mStencilBackPassOp; }
void          GraphicsState::setStencilBackDepthFailOp(vk::StencilOp val) {
  mStencilBackDepthFailOp = val;
  mDirty                  = true;
}
vk::StencilOp GraphicsState::getStencilBackDepthFailOp() const { return mStencilBackDepthFailOp; }
void          GraphicsState::setStencilBackCompareOp(vk::CompareOp val) {
  mStencilBackCompareOp = val;
  mDirty                = true;
}
vk::CompareOp GraphicsState::getStencilBackCompareOp() const { return mStencilBackCompareOp; }
void          GraphicsState::setStencilBackCompareMask(uint32_t val) {
  mStencilBackCompareMask = val;
  mDirty                  = true;
}
uint32_t GraphicsState::getStencilBackCompareMask() const { return mStencilBackCompareMask; }
void     GraphicsState::setStencilBackWriteMask(uint32_t val) {
  mStencilBackWriteMask = val;
  mDirty                = true;
}
uint32_t GraphicsState::getStencilBackWriteMask() const { return mStencilBackWriteMask; }
void     GraphicsState::setStencilBackReference(uint32_t val) {
  mStencilBackReference = val;
  mDirty                = true;
}
uint32_t GraphicsState::getStencilBackReference() const { return mStencilBackReference; }
void     GraphicsState::setMinDepthBounds(float val) {
  mMinDepthBounds = val;
  mDirty          = true;
}
float GraphicsState::getMinDepthBounds() const { return mMinDepthBounds; }
void  GraphicsState::setMaxDepthBounds(float val) {
  mMaxDepthBounds = val;
  mDirty          = true;
}
float GraphicsState::getMaxDepthBounds() const { return mMaxDepthBounds; }

// Input Assembly State --------------------------------------------------------------------------
void GraphicsState::setTopology(vk::PrimitiveTopology val) {
  mTopology = val;
  mDirty    = true;
}
vk::PrimitiveTopology GraphicsState::getTopology() const { return mTopology; }
void                  GraphicsState::setPrimitiveRestartEnable(bool val) {
  mPrimitiveRestartEnable = val;
  mDirty                  = true;
}
bool GraphicsState::getPrimitiveRestartEnable() const { return mPrimitiveRestartEnable; }

// Multisample State -----------------------------------------------------------------------------
void GraphicsState::setRasterizationSamples(vk::SampleCountFlagBits val) {
  mRasterizationSamples = val;
  mDirty                = true;
}
vk::SampleCountFlagBits GraphicsState::getRasterizationSamples() const {
  return mRasterizationSamples;
}
void GraphicsState::setSampleShadingEnable(bool val) {
  mSampleShadingEnable = val;
  mDirty               = true;
}
bool GraphicsState::getSampleShadingEnable() const { return mSampleShadingEnable; }
void GraphicsState::setMinSampleShading(float val) {
  mMinSampleShading = val;
  mDirty            = true;
}
float GraphicsState::getMinSampleShading() const { return mMinSampleShading; }
void  GraphicsState::setAlphaToCoverageEnable(bool val) {
  mAlphaToCoverageEnable = val;
  mDirty                 = true;
}
bool GraphicsState::getAlphaToCoverageEnable() const { return mAlphaToCoverageEnable; }
void GraphicsState::setAlphaToOneEnable(bool val) {
  mAlphaToOneEnable = val;
  mDirty            = true;
}
bool GraphicsState::getAlphaToOneEnable() const { return mAlphaToOneEnable; }
void GraphicsState::setSampleMask(std::vector<uint32_t> val) {
  mSampleMask = val;
  mDirty      = true;
}
std::vector<uint32_t> GraphicsState::getSampleMask() const { return mSampleMask; }

// Rasterization State ---------------------------------------------------------------------------
void GraphicsState::setDepthClampEnable(bool val) {
  mDepthClampEnable = val;
  mDirty            = true;
}
bool GraphicsState::getDepthClampEnable() const { return mDepthClampEnable; }
void GraphicsState::setRasterizerDiscardEnable(bool val) {
  mRasterizerDiscardEnable = val;
  mDirty                   = true;
}
bool GraphicsState::getRasterizerDiscardEnable() const { return mRasterizerDiscardEnable; }
void GraphicsState::setPolygonMode(vk::PolygonMode val) {
  mPolygonMode = val;
  mDirty       = true;
}
vk::PolygonMode GraphicsState::getPolygonMode() const { return mPolygonMode; }
void            GraphicsState::setCullMode(vk::CullModeFlags val) {
  mCullMode = val;
  mDirty    = true;
}
vk::CullModeFlags GraphicsState::getCullMode() const { return mCullMode; }
void              GraphicsState::setFrontFace(vk::FrontFace val) {
  mFrontFace = val;
  mDirty     = true;
}
vk::FrontFace GraphicsState::getFrontFace() const { return mFrontFace; }
void          GraphicsState::setDepthBiasEnable(bool val) {
  mDepthBiasEnable = val;
  mDirty           = true;
}
bool GraphicsState::getDepthBiasEnable() const { return mDepthBiasEnable; }
void GraphicsState::setDepthBiasConstantFactor(float val) {
  mDepthBiasConstantFactor = val;
  mDirty                   = true;
}
float GraphicsState::getDepthBiasConstantFactor() const { return mDepthBiasConstantFactor; }
void  GraphicsState::setDepthBiasClamp(float val) {
  mDepthBiasClamp = val;
  mDirty          = true;
}
float GraphicsState::getDepthBiasClamp() const { return mDepthBiasClamp; }
void  GraphicsState::setDepthBiasSlopeFactor(float val) {
  mDepthBiasSlopeFactor = val;
  mDirty                = true;
}
float GraphicsState::getDepthBiasSlopeFactor() const { return mDepthBiasSlopeFactor; }
void  GraphicsState::setLineWidth(float val) {
  mLineWidth = val;
  mDirty     = true;
}
float GraphicsState::getLineWidth() const { return mLineWidth; }

// Tesselation State -----------------------------------------------------------------------------
void GraphicsState::setTessellationPatchControlPoints(uint32_t val) {
  mTessellationPatchControlPoints = val;
  mDirty                          = true;
}
uint32_t GraphicsState::getTessellationPatchControlPoints() const {
  return mTessellationPatchControlPoints;
}

// Vertex Input State ----------------------------------------------------------------------------
void GraphicsState::addVertexInputBinding(vk::VertexInputBindingDescription const& val) {
  mVertexInputBindings.push_back(val);
  mDirty = true;
}
void GraphicsState::setVertexInputBindings(
  std::vector<vk::VertexInputBindingDescription> const& val) {
  mVertexInputBindings = val;
  mDirty               = true;
}
std::vector<vk::VertexInputBindingDescription> const&
GraphicsState::getVertexInputBindings() const {
  return mVertexInputBindings;
}
void GraphicsState::addVertexInputAttribute(vk::VertexInputAttributeDescription const& val) {
  mVertexInputAttributes.push_back(val);
  mDirty = true;
}

void GraphicsState::setVertexInputAttributes(
  std::vector<vk::VertexInputAttributeDescription> const& val) {
  mVertexInputAttributes = val;
  mDirty                 = true;
}
std::vector<vk::VertexInputAttributeDescription> const&
GraphicsState::getVertexInputAttributes() const {
  return mVertexInputAttributes;
}

// Viewport State --------------------------------------------------------------------------------
void GraphicsState::addViewport(Viewport const& val) {
  mViewports.push_back(val);
  mDirty = true;
}
void GraphicsState::setViewports(std::vector<Viewport> const& val) {
  mViewports = val;
  mDirty     = true;
}
std::vector<GraphicsState::Viewport> const& GraphicsState::getViewports() const {
  return mViewports;
}

void GraphicsState::addScissor(Scissor const& val) {
  mScissors.push_back(val);
  mDirty = true;
}
void GraphicsState::setScissors(std::vector<Scissor> const& val) {
  mScissors = val;
  mDirty    = true;
}
std::vector<GraphicsState::Scissor> const& GraphicsState::getScissors() const { return mScissors; }

// Dynamic State ---------------------------------------------------------------------------------
void GraphicsState::addDynamicState(vk::DynamicState val) {
  mDynamicState.insert(val);
  mDirty = true;
}
void GraphicsState::removeDynamicState(vk::DynamicState val) {
  mDynamicState.erase(val);
  mDirty = true;
}
void GraphicsState::setDynamicState(std::set<vk::DynamicState> const& val) {
  mDynamicState = val;
  mDirty        = true;
}
std::set<vk::DynamicState> const& GraphicsState::getDynamicState() const { return mDynamicState; }

////////////////////////////////////////////////////////////////////////////////////////////////////

Core::BitHash GraphicsState::getHash() const {

  if (mDirty) {
    mHash.clear();

    mHash.push<1>(mBlendLogicOpEnable);
    mHash.push<4>(mBlendLogicOp);
    for (auto const& attachmentState : mBlendAttachments) {
      mHash.push<1>(attachmentState.mBlendEnable);
      mHash.push<5>(attachmentState.mSrcColorBlendFactor);
      mHash.push<5>(attachmentState.mDstColorBlendFactor);
      mHash.push<3>(attachmentState.mColorBlendOp);
      mHash.push<5>(attachmentState.mSrcAlphaBlendFactor);
      mHash.push<5>(attachmentState.mDstAlphaBlendFactor);
      mHash.push<3>(attachmentState.mAlphaBlendOp);
      mHash.push<4>(attachmentState.mColorWriteMask);
    }
    if (!getDynamicState().count(vk::DynamicState::eBlendConstants)) {
      mHash.push<32>(mBlendConstants[0]);
      mHash.push<32>(mBlendConstants[1]);
      mHash.push<32>(mBlendConstants[2]);
      mHash.push<32>(mBlendConstants[3]);
    }

    mHash.push<1>(mDepthTestEnable);
    mHash.push<1>(mDepthWriteEnable);
    mHash.push<3>(mDepthCompareOp);
    mHash.push<1>(mDepthBoundsTestEnable);
    mHash.push<1>(mStencilTestEnable);
    mHash.push<3>(mStencilFrontFailOp);
    mHash.push<3>(mStencilFrontPassOp);
    mHash.push<3>(mStencilFrontDepthFailOp);
    mHash.push<3>(mStencilFrontCompareOp);
    if (!getDynamicState().count(vk::DynamicState::eStencilCompareMask)) {
      mHash.push<32>(mStencilFrontCompareMask);
    }
    if (!getDynamicState().count(vk::DynamicState::eStencilWriteMask)) {
      mHash.push<32>(mStencilFrontWriteMask);
    }
    if (!getDynamicState().count(vk::DynamicState::eStencilReference)) {
      mHash.push<32>(mStencilFrontReference);
    }
    mHash.push<3>(mStencilBackFailOp);
    mHash.push<3>(mStencilBackPassOp);
    mHash.push<3>(mStencilBackDepthFailOp);
    mHash.push<3>(mStencilBackCompareOp);
    if (!getDynamicState().count(vk::DynamicState::eStencilCompareMask)) {
      mHash.push<32>(mStencilBackCompareMask);
    }
    if (!getDynamicState().count(vk::DynamicState::eStencilWriteMask)) {
      mHash.push<32>(mStencilBackWriteMask);
    }
    if (!getDynamicState().count(vk::DynamicState::eStencilReference)) {
      mHash.push<32>(mStencilBackReference);
    }
    if (!getDynamicState().count(vk::DynamicState::eDepthBounds)) {
      mHash.push<32>(mMinDepthBounds);
      mHash.push<32>(mMaxDepthBounds);
    }

    for (auto const& dynamicState : mDynamicState) {
      mHash.push<32>(dynamicState);
    }

    mHash.push<4>(mTopology);
    mHash.push<1>(mPrimitiveRestartEnable);

    mHash.push<3>(mRasterizationSamples);
    mHash.push<1>(mSampleShadingEnable);
    mHash.push<32>(mMinSampleShading);
    for (uint32_t mask : mSampleMask) {
      mHash.push<32>(mask);
    }
    mHash.push<1>(mAlphaToCoverageEnable);
    mHash.push<1>(mAlphaToOneEnable);

    mHash.push<1>(mDepthClampEnable);
    mHash.push<1>(mRasterizerDiscardEnable);
    mHash.push<2>(mPolygonMode);
    mHash.push<2>(mCullMode);
    mHash.push<1>(mFrontFace);
    mHash.push<1>(mDepthBiasEnable);
    if (!getDynamicState().count(vk::DynamicState::eDepthBias)) {
      mHash.push<32>(mDepthBiasConstantFactor);
      mHash.push<32>(mDepthBiasClamp);
      mHash.push<32>(mDepthBiasSlopeFactor);
    }
    if (!getDynamicState().count(vk::DynamicState::eLineWidth)) {
      mHash.push<32>(mLineWidth);
    }

    mHash.push<32>(mTessellationPatchControlPoints);

    for (auto const& binding : mVertexInputBindings) {
      mHash.push<32>(binding.binding);
      mHash.push<32>(binding.stride);
      mHash.push<1>(binding.inputRate);
    }
    for (auto const& attribute : mVertexInputAttributes) {
      mHash.push<32>(attribute.location);
      mHash.push<32>(attribute.binding);
      mHash.push<32>(attribute.format);
      mHash.push<32>(attribute.offset);
    }

    if (getDynamicState().count(vk::DynamicState::eViewport)) {
      mHash.push<32>(mViewports.size());
    } else {
      for (auto const& viewport : mViewports) {
        mHash.push<32>(viewport.mOffset[0]);
        mHash.push<32>(viewport.mOffset[1]);
        mHash.push<32>(viewport.mExtend[0]);
        mHash.push<32>(viewport.mExtend[1]);
        mHash.push<32>(viewport.mMinDepth);
        mHash.push<32>(viewport.mMaxDepth);
      }
    }
    if (getDynamicState().count(vk::DynamicState::eScissor)) {
      mHash.push<32>(mScissors.size());
    } else {
      for (auto const& scissor : mScissors) {
        mHash.push<32>(scissor.mOffset[0]);
        mHash.push<32>(scissor.mOffset[1]);
        mHash.push<32>(scissor.mExtend[0]);
        mHash.push<32>(scissor.mExtend[1]);
      }
    }

    mDirty = false;
  }

  return mHash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
