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
#include "PipelineFactory.hpp"

#include "../Core/Logger.hpp"
#include "Context.hpp"
#include "GraphicsState.hpp"
#include "ShaderModule.hpp"
#include "ShaderProgram.hpp"

#include <iostream>

namespace Illusion::Graphics {

PipelineFactory::PipelineFactory(std::shared_ptr<Context> const& context)
  : mContext(context) {}

PipelineFactory::~PipelineFactory() {}

std::shared_ptr<vk::Pipeline> PipelineFactory::createPipeline(
  GraphicsState const& gs, vk::RenderPass const& renderpass, uint32_t subPass) {

  Core::BitHash hash = gs.getHash();
  hash.push<32>(subPass);

  auto cached = mCache.find(hash);
  if (cached != mCache.end()) { return cached->second; }

  // -----------------------------------------------------------------------------------------------
  std::vector<vk::PipelineShaderStageCreateInfo> stageInfos;
  if (gs.getShaderProgram()) {
    for (auto const& i : gs.getShaderProgram()->getModules()) {
      vk::PipelineShaderStageCreateInfo stageInfo;
      stageInfo.stage               = i->getStage();
      stageInfo.module              = *i->getModule();
      stageInfo.pName               = "main";
      stageInfo.pSpecializationInfo = nullptr;
      stageInfos.push_back(stageInfo);
    }
  }

  // -----------------------------------------------------------------------------------------------
  vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;

  std::vector<vk::VertexInputBindingDescription>   vertexInputBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
  for (auto const& i : gs.getVertexInputState().mBindings) {
    vertexInputBindingDescriptions.push_back({i.mBinding, i.mStride, i.mInputRate});
  }
  for (auto const& i : gs.getVertexInputState().mAttributes) {
    vertexInputAttributeDescriptions.push_back({i.mLocation, i.mBinding, i.mFormat, i.mOffset});
  }
  vertexInputStateInfo.vertexBindingDescriptionCount   = vertexInputBindingDescriptions.size();
  vertexInputStateInfo.pVertexBindingDescriptions      = vertexInputBindingDescriptions.data();
  vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
  vertexInputStateInfo.pVertexAttributeDescriptions    = vertexInputAttributeDescriptions.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
  inputAssemblyStateInfo.topology = gs.getInputAssemblyState().mTopology;
  inputAssemblyStateInfo.primitiveRestartEnable =
    gs.getInputAssemblyState().mPrimitiveRestartEnable;

  // -----------------------------------------------------------------------------------------------
  vk::PipelineTessellationStateCreateInfo tessellationStateInfo;
  tessellationStateInfo.patchControlPoints = gs.getTessellationState().mPatchControlPoints;

  // -----------------------------------------------------------------------------------------------
  vk::PipelineViewportStateCreateInfo viewportStateInfo;
  std::vector<vk::Viewport>           viewports;
  std::vector<vk::Rect2D>             scissors;
  for (auto const& i : gs.getViewportState().mViewports) {
    viewports.push_back(
      {i.mOffset[0], i.mOffset[1], i.mExtend[0], i.mExtend[1], i.mMinDepth, i.mMaxDepth});
  }
  for (auto const& i : gs.getViewportState().mScissors) {
    scissors.push_back({{i.mOffset[0], i.mOffset[1]}, {i.mExtend[0], i.mExtend[1]}});
  }
  viewportStateInfo.viewportCount = viewports.size();
  viewportStateInfo.pViewports    = viewports.data();
  viewportStateInfo.scissorCount  = scissors.size();
  viewportStateInfo.pScissors     = scissors.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;
  rasterizationStateInfo.depthClampEnable = gs.getRasterizationState().mDepthClampEnable;
  rasterizationStateInfo.rasterizerDiscardEnable =
    gs.getRasterizationState().mRasterizerDiscardEnable;
  rasterizationStateInfo.polygonMode     = gs.getRasterizationState().mPolygonMode;
  rasterizationStateInfo.cullMode        = gs.getRasterizationState().mCullMode;
  rasterizationStateInfo.frontFace       = gs.getRasterizationState().mFrontFace;
  rasterizationStateInfo.depthBiasEnable = gs.getRasterizationState().mDepthBiasEnable;
  rasterizationStateInfo.depthBiasConstantFactor =
    gs.getRasterizationState().mDepthBiasConstantFactor;
  rasterizationStateInfo.depthBiasClamp       = gs.getRasterizationState().mDepthBiasClamp;
  rasterizationStateInfo.depthBiasSlopeFactor = gs.getRasterizationState().mDepthBiasSlopeFactor;
  rasterizationStateInfo.lineWidth            = gs.getRasterizationState().mLineWidth;

  // -----------------------------------------------------------------------------------------------
  vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
  multisampleStateInfo.rasterizationSamples  = gs.getMultisampleState().mRasterizationSamples;
  multisampleStateInfo.sampleShadingEnable   = gs.getMultisampleState().mSampleShadingEnable;
  multisampleStateInfo.minSampleShading      = gs.getMultisampleState().mMinSampleShading;
  multisampleStateInfo.pSampleMask           = gs.getMultisampleState().mSampleMask.data();
  multisampleStateInfo.alphaToCoverageEnable = gs.getMultisampleState().mAlphaToCoverageEnable;
  multisampleStateInfo.alphaToOneEnable      = gs.getMultisampleState().mAlphaToOneEnable;

  // -----------------------------------------------------------------------------------------------
  vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo;
  depthStencilStateInfo.depthTestEnable       = gs.getDepthStencilState().mDepthTestEnable;
  depthStencilStateInfo.depthWriteEnable      = gs.getDepthStencilState().mDepthWriteEnable;
  depthStencilStateInfo.depthCompareOp        = gs.getDepthStencilState().mDepthCompareOp;
  depthStencilStateInfo.depthBoundsTestEnable = gs.getDepthStencilState().mDepthBoundsTestEnable;
  depthStencilStateInfo.stencilTestEnable     = gs.getDepthStencilState().mStencilTestEnable;
  depthStencilStateInfo.front                 = {gs.getDepthStencilState().mFront.mFailOp,
                                 gs.getDepthStencilState().mFront.mPassOp,
                                 gs.getDepthStencilState().mFront.mDepthFailOp,
                                 gs.getDepthStencilState().mFront.mCompareOp,
                                 gs.getDepthStencilState().mFront.mCompareMask,
                                 gs.getDepthStencilState().mFront.mWriteMask,
                                 gs.getDepthStencilState().mFront.mReference};
  depthStencilStateInfo.back                  = {gs.getDepthStencilState().mBack.mFailOp,
                                gs.getDepthStencilState().mBack.mPassOp,
                                gs.getDepthStencilState().mBack.mDepthFailOp,
                                gs.getDepthStencilState().mBack.mCompareOp,
                                gs.getDepthStencilState().mBack.mCompareMask,
                                gs.getDepthStencilState().mBack.mWriteMask,
                                gs.getDepthStencilState().mBack.mReference};
  depthStencilStateInfo.minDepthBounds        = gs.getDepthStencilState().mMinDepthBounds;
  depthStencilStateInfo.maxDepthBounds        = gs.getDepthStencilState().mMaxDepthBounds;

  // -----------------------------------------------------------------------------------------------
  vk::PipelineColorBlendStateCreateInfo              colorBlendStateInfo;
  std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachments;
  for (auto const& i : gs.getColorBlendState().mAttachments) {
    pipelineColorBlendAttachments.push_back({i.mBlendEnable,
                                             i.mSrcColorBlendFactor,
                                             i.mDstColorBlendFactor,
                                             i.mColorBlendOp,
                                             i.mSrcAlphaBlendFactor,
                                             i.mDstAlphaBlendFactor,
                                             i.mAlphaBlendOp,
                                             i.mColorWriteMask});
  }
  colorBlendStateInfo.logicOpEnable     = gs.getColorBlendState().mLogicOpEnable;
  colorBlendStateInfo.logicOp           = gs.getColorBlendState().mLogicOp;
  colorBlendStateInfo.attachmentCount   = pipelineColorBlendAttachments.size();
  colorBlendStateInfo.pAttachments      = pipelineColorBlendAttachments.data();
  colorBlendStateInfo.blendConstants[0] = gs.getColorBlendState().mBlendConstants[0];
  colorBlendStateInfo.blendConstants[1] = gs.getColorBlendState().mBlendConstants[1];
  colorBlendStateInfo.blendConstants[2] = gs.getColorBlendState().mBlendConstants[2];
  colorBlendStateInfo.blendConstants[3] = gs.getColorBlendState().mBlendConstants[3];

  // -----------------------------------------------------------------------------------------------
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  std::vector<vk::DynamicState>      dynamicState(
    gs.getDynamicState().begin(), gs.getDynamicState().end());
  dynamicStateInfo.dynamicStateCount = dynamicState.size();
  dynamicStateInfo.pDynamicStates    = dynamicState.data();

  // -----------------------------------------------------------------------------------------------
  vk::GraphicsPipelineCreateInfo info;
  info.stageCount          = stageInfos.size();
  info.pStages             = stageInfos.data();
  info.pVertexInputState   = &vertexInputStateInfo;
  info.pInputAssemblyState = &inputAssemblyStateInfo;
  info.pTessellationState  = &tessellationStateInfo;
  info.pViewportState      = &viewportStateInfo;
  info.pRasterizationState = &rasterizationStateInfo;
  info.pMultisampleState   = &multisampleStateInfo;
  info.pDepthStencilState  = &depthStencilStateInfo;
  info.pColorBlendState    = &colorBlendStateInfo;
  if (gs.getDynamicState().size() > 0) { info.pDynamicState = &dynamicStateInfo; }
  info.renderPass = renderpass;
  info.subpass    = subPass;

  if (gs.getShaderProgram()) { info.layout = *gs.getShaderProgram()->getPipelineLayout(); }

  auto pipeline = mContext->createPipeline(info);
  mCache[hash]  = pipeline;

  return pipeline;
}

void PipelineFactory::clearCache() { mCache.clear(); }

} // namespace Illusion::Graphics
