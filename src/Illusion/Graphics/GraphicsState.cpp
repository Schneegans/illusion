////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "GraphicsState.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsState::setPipeline(std::shared_ptr<Pipeline> const& pipeline) {
  mPipeline = pipeline;
  mDirty    = true;
}

void GraphicsState::setDynamicState(std::vector<vk::DynamicState> const& dynamicState) {
  mDynamicState = dynamicState;
  mDirty        = true;
}

void GraphicsState::setVertexInputState(VertexInputState const& vertexInputState) {
  mVertexInputState = vertexInputState;
  mDirty            = true;
}

void GraphicsState::setInputAssemblyState(InputAssemblyState const& inputAssemblyState) {
  mInputAssemblyState = inputAssemblyState;
  mDirty              = true;
}

void GraphicsState::setTessellationState(TessellationState const& tessellationState) {
  mTessellationState = tessellationState;
  mDirty             = true;
}

void GraphicsState::setViewportState(ViewportState const& viewportState) {
  mViewportState = viewportState;
  mDirty         = true;
}

void GraphicsState::setRasterizationState(RasterizationState const& rasterizationState) {
  mRasterizationState = rasterizationState;
  mDirty              = true;
}

void GraphicsState::setMultisampleState(MultisampleState const& multisampleState) {
  mMultisampleState = multisampleState;
  mDirty            = true;
}

void GraphicsState::setDepthStencilState(DepthStencilState const& depthStencilState) {
  mDepthStencilState = depthStencilState;
  mDirty             = true;
}

void GraphicsState::setColorBlendState(ColorBlendState const& colorBlendState) {
  mColorBlendState = colorBlendState;
  mDirty           = true;
}

std::shared_ptr<Pipeline> const& GraphicsState::getPipeline() const { return mPipeline; }

std::vector<vk::DynamicState> const& GraphicsState::getDynamicState() const {
  return mDynamicState;
}

GraphicsState::VertexInputState const& GraphicsState::getVertexInputState() const {
  return mVertexInputState;
}

GraphicsState::InputAssemblyState const& GraphicsState::getInputAssemblyState() const {
  return mInputAssemblyState;
}

GraphicsState::TessellationState const& GraphicsState::getTessellationState() const {
  return mTessellationState;
}

GraphicsState::ViewportState const& GraphicsState::getViewportState() const {
  return mViewportState;
}

GraphicsState::RasterizationState const& GraphicsState::getRasterizationState() const {
  return mRasterizationState;
}

GraphicsState::MultisampleState const& GraphicsState::getMultisampleState() const {
  return mMultisampleState;
}

GraphicsState::DepthStencilState const& GraphicsState::getDepthStencilState() const {
  return mDepthStencilState;
}

GraphicsState::ColorBlendState const& GraphicsState::getColorBlendState() const {
  return mColorBlendState;
}

Core::BitHash const& GraphicsState::getHash() const {
  if (mDirty) {

    mHash.clear();

    mHash.push<64>(mPipeline.get());

    for (auto const& dynamicState : mDynamicState) {
      mHash.push<32>(dynamicState);
    }

    mHash.push<1>(mRasterizationState.mDepthClampEnable);
    mHash.push<1>(mRasterizationState.mRasterizerDiscardEnable);
    mHash.push<2>(mRasterizationState.mPolygonMode);
    mHash.push<2>(mRasterizationState.mCullMode);
    mHash.push<1>(mRasterizationState.mFrontFace);
    mHash.push<1>(mRasterizationState.mDepthBiasEnable);
    mHash.push<32>(mRasterizationState.mDepthBiasConstantFactor);
    mHash.push<32>(mRasterizationState.mDepthBiasClamp);
    mHash.push<32>(mRasterizationState.mDepthBiasSlopeFactor);
    mHash.push<32>(mRasterizationState.mLineWidth);

    mHash.push<1>(mDepthStencilState.mDepthTestEnable);
    mHash.push<1>(mDepthStencilState.mDepthWriteEnable);
    mHash.push<3>(mDepthStencilState.mDepthCompareOp);
    mHash.push<1>(mDepthStencilState.mDepthBoundsTestEnable);
    mHash.push<1>(mDepthStencilState.mStencilTestEnable);
    mHash.push<3>(mDepthStencilState.mFront.mFailOp);
    mHash.push<3>(mDepthStencilState.mFront.mPassOp);
    mHash.push<3>(mDepthStencilState.mFront.mDepthFailOp);
    mHash.push<3>(mDepthStencilState.mFront.mCompareOp);
    mHash.push<32>(mDepthStencilState.mFront.mCompareMask);
    mHash.push<32>(mDepthStencilState.mFront.mWriteMask);
    mHash.push<32>(mDepthStencilState.mFront.mReference);
    mHash.push<3>(mDepthStencilState.mBack.mFailOp);
    mHash.push<3>(mDepthStencilState.mBack.mPassOp);
    mHash.push<3>(mDepthStencilState.mBack.mDepthFailOp);
    mHash.push<3>(mDepthStencilState.mBack.mCompareOp);
    mHash.push<32>(mDepthStencilState.mBack.mCompareMask);
    mHash.push<32>(mDepthStencilState.mBack.mWriteMask);
    mHash.push<32>(mDepthStencilState.mBack.mReference);
    mHash.push<32>(mDepthStencilState.mMinDepthBounds);
    mHash.push<32>(mDepthStencilState.mMaxDepthBounds);

    mHash.push<1>(mColorBlendState.mLogicOpEnable);
    mHash.push<4>(mColorBlendState.mLogicOp);
    for (auto const& attachmentState : mColorBlendState.mAttachments) {
      mHash.push<1>(attachmentState.mBlendEnable);
      mHash.push<5>(attachmentState.mSrcColorBlendFactor);
      mHash.push<5>(attachmentState.mDstColorBlendFactor);
      mHash.push<3>(attachmentState.mColorBlendOp);
      mHash.push<5>(attachmentState.mSrcAlphaBlendFactor);
      mHash.push<5>(attachmentState.mDstAlphaBlendFactor);
      mHash.push<3>(attachmentState.mAlphaBlendOp);
      mHash.push<4>(attachmentState.mColorWriteMask);
    }
    mHash.push<32>(mColorBlendState.mBlendConstants[0]);
    mHash.push<32>(mColorBlendState.mBlendConstants[1]);
    mHash.push<32>(mColorBlendState.mBlendConstants[2]);
    mHash.push<32>(mColorBlendState.mBlendConstants[3]);

    mHash.push<3>(mMultisampleState.mRasterizationSamples);
    mHash.push<1>(mMultisampleState.mSampleShadingEnable);
    mHash.push<32>(mMultisampleState.mMinSampleShading);
    for (uint32_t mask : mMultisampleState.mSampleMask) {
      mHash.push<32>(mask);
    }
    mHash.push<1>(mMultisampleState.mAlphaToCoverageEnable);
    mHash.push<1>(mMultisampleState.mAlphaToOneEnable);

    mHash.push<32>(mTessellationState.mPatchControlPoints);

    mHash.push<4>(mInputAssemblyState.mTopology);
    mHash.push<1>(mInputAssemblyState.mPrimitiveRestartEnable);

    for (auto const& binding : mVertexInputState.mBindings) {
      mHash.push<32>(binding.mBinding);
      mHash.push<32>(binding.mStride);
      mHash.push<1>(binding.mInputRate);
    }
    for (auto const& attribute : mVertexInputState.mAttributes) {
      mHash.push<32>(attribute.mLocation);
      mHash.push<32>(attribute.mBinding);
      mHash.push<32>(attribute.mFormat);
      mHash.push<32>(attribute.mOffset);
    }

    for (auto const& viewport : mViewportState.mViewports) {
      mHash.push<32>(viewport.mOffset[0]);
      mHash.push<32>(viewport.mOffset[1]);
      mHash.push<32>(viewport.mExtend[0]);
      mHash.push<32>(viewport.mExtend[1]);
      mHash.push<32>(viewport.mMinDepth);
      mHash.push<32>(viewport.mMaxDepth);
    }
    for (auto const& scissor : mViewportState.mScissors) {
      mHash.push<32>(scissor.mOffset[0]);
      mHash.push<32>(scissor.mOffset[1]);
      mHash.push<32>(scissor.mExtend[0]);
      mHash.push<32>(scissor.mExtend[1]);
    }

    mDirty = false;
  }
  return mHash;
}

} // namespace Illusion::Graphics
