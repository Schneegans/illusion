////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PipelineCache.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "GraphicsState.hpp"
#include "PipelineReflection.hpp"
#include "ShaderModule.hpp"
#include "ShaderProgram.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineCache::PipelineCache(std::shared_ptr<Device> const& device)
  : mDevice(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineCache::~PipelineCache() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Pipeline> PipelineCache::getPipelineHandle(
  GraphicsState const& gs, vk::RenderPass const& renderpass, uint32_t subPass) {

  Core::BitHash hash = gs.getHash();
  hash.push<32>(subPass);

  {
    std::unique_lock<std::mutex> lock(mMutex);
    auto                         cached = mCache.find(hash);
    if (cached != mCache.end()) {
      return cached->second;
    }
  }

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
  for (auto const& i : gs.getVertexInputBindings()) {
    vertexInputBindingDescriptions.push_back({i.binding, i.stride, i.inputRate});
  }
  for (auto const& i : gs.getVertexInputAttributes()) {
    vertexInputAttributeDescriptions.push_back({i.location, i.binding, i.format, i.offset});
  }
  vertexInputStateInfo.vertexBindingDescriptionCount   = vertexInputBindingDescriptions.size();
  vertexInputStateInfo.pVertexBindingDescriptions      = vertexInputBindingDescriptions.data();
  vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
  vertexInputStateInfo.pVertexAttributeDescriptions    = vertexInputAttributeDescriptions.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
  inputAssemblyStateInfo.topology               = gs.getTopology();
  inputAssemblyStateInfo.primitiveRestartEnable = gs.getPrimitiveRestartEnable();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineTessellationStateCreateInfo tessellationStateInfo;
  tessellationStateInfo.patchControlPoints = gs.getTessellationPatchControlPoints();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineViewportStateCreateInfo viewportStateInfo;
  std::vector<vk::Viewport>           viewports;
  std::vector<vk::Rect2D>             scissors;
  for (auto const& i : gs.getViewports()) {
    viewports.push_back(
      {i.mOffset[0], i.mOffset[1], i.mExtend[0], i.mExtend[1], i.mMinDepth, i.mMaxDepth});
  }

  // use viewport as scissors if no scissors are defined
  if (gs.getScissors().size() > 0) {
    for (auto const& i : gs.getScissors()) {
      scissors.push_back({{i.mOffset[0], i.mOffset[1]}, {i.mExtend[0], i.mExtend[1]}});
    }
  } else {
    for (auto const& i : gs.getViewports()) {
      scissors.push_back({{(int32_t)i.mOffset[0], (int32_t)i.mOffset[1]},
                          {(uint32_t)i.mExtend[0], (uint32_t)i.mExtend[1]}});
    }
  }
  viewportStateInfo.viewportCount = viewports.size();
  viewportStateInfo.pViewports    = viewports.data();
  viewportStateInfo.scissorCount  = scissors.size();
  viewportStateInfo.pScissors     = scissors.data();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;
  rasterizationStateInfo.depthClampEnable        = gs.getDepthClampEnable();
  rasterizationStateInfo.rasterizerDiscardEnable = gs.getRasterizerDiscardEnable();
  rasterizationStateInfo.polygonMode             = gs.getPolygonMode();
  rasterizationStateInfo.cullMode                = gs.getCullMode();
  rasterizationStateInfo.frontFace               = gs.getFrontFace();
  rasterizationStateInfo.depthBiasEnable         = gs.getDepthBiasEnable();
  rasterizationStateInfo.depthBiasConstantFactor = gs.getDepthBiasConstantFactor();
  rasterizationStateInfo.depthBiasClamp          = gs.getDepthBiasClamp();
  rasterizationStateInfo.depthBiasSlopeFactor    = gs.getDepthBiasSlopeFactor();
  rasterizationStateInfo.lineWidth               = gs.getLineWidth();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
  multisampleStateInfo.rasterizationSamples  = gs.getRasterizationSamples();
  multisampleStateInfo.sampleShadingEnable   = gs.getSampleShadingEnable();
  multisampleStateInfo.minSampleShading      = gs.getMinSampleShading();
  multisampleStateInfo.pSampleMask           = gs.getSampleMask().data();
  multisampleStateInfo.alphaToCoverageEnable = gs.getAlphaToCoverageEnable();
  multisampleStateInfo.alphaToOneEnable      = gs.getAlphaToOneEnable();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo;
  depthStencilStateInfo.depthTestEnable       = gs.getDepthTestEnable();
  depthStencilStateInfo.depthWriteEnable      = gs.getDepthWriteEnable();
  depthStencilStateInfo.depthCompareOp        = gs.getDepthCompareOp();
  depthStencilStateInfo.depthBoundsTestEnable = gs.getDepthBoundsTestEnable();
  depthStencilStateInfo.stencilTestEnable     = gs.getStencilTestEnable();
  depthStencilStateInfo.front                 = {gs.getStencilFrontFailOp(),
                                 gs.getStencilFrontPassOp(),
                                 gs.getStencilFrontDepthFailOp(),
                                 gs.getStencilFrontCompareOp(),
                                 gs.getStencilFrontCompareMask(),
                                 gs.getStencilFrontWriteMask(),
                                 gs.getStencilFrontReference()};
  depthStencilStateInfo.back                  = {gs.getStencilBackFailOp(),
                                gs.getStencilBackPassOp(),
                                gs.getStencilBackDepthFailOp(),
                                gs.getStencilBackCompareOp(),
                                gs.getStencilBackCompareMask(),
                                gs.getStencilBackWriteMask(),
                                gs.getStencilBackReference()};
  depthStencilStateInfo.minDepthBounds        = gs.getMinDepthBounds();
  depthStencilStateInfo.maxDepthBounds        = gs.getMaxDepthBounds();

  // -----------------------------------------------------------------------------------------------
  vk::PipelineColorBlendStateCreateInfo              colorBlendStateInfo;
  std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachments;
  for (auto const& i : gs.getBlendAttachments()) {
    pipelineColorBlendAttachments.push_back({i.mBlendEnable,
                                             i.mSrcColorBlendFactor,
                                             i.mDstColorBlendFactor,
                                             i.mColorBlendOp,
                                             i.mSrcAlphaBlendFactor,
                                             i.mDstAlphaBlendFactor,
                                             i.mAlphaBlendOp,
                                             i.mColorWriteMask});
  }
  colorBlendStateInfo.logicOpEnable     = gs.getBlendLogicOpEnable();
  colorBlendStateInfo.logicOp           = gs.getBlendLogicOp();
  colorBlendStateInfo.attachmentCount   = pipelineColorBlendAttachments.size();
  colorBlendStateInfo.pAttachments      = pipelineColorBlendAttachments.data();
  colorBlendStateInfo.blendConstants[0] = gs.getBlendConstants()[0];
  colorBlendStateInfo.blendConstants[1] = gs.getBlendConstants()[1];
  colorBlendStateInfo.blendConstants[2] = gs.getBlendConstants()[2];
  colorBlendStateInfo.blendConstants[3] = gs.getBlendConstants()[3];

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
  if (gs.getDynamicState().size() > 0) {
    info.pDynamicState = &dynamicStateInfo;
  }
  info.renderPass = renderpass;
  info.subpass    = subPass;

  if (gs.getShaderProgram()) {
    info.layout = *gs.getShaderProgram()->getReflection()->getLayout();
  }

  auto pipeline = mDevice->createPipeline(info);

  std::unique_lock<std::mutex> lock(mMutex);
  mCache[hash] = pipeline;

  return pipeline;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PipelineCache::clear() {
  std::unique_lock<std::mutex> lock(mMutex);
  mCache.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
