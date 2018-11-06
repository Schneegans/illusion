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
#include "Material.hpp"

#include "../Core/Logger.hpp"
#include "Engine.hpp"
#include "PipelineLayout.hpp"
#include "RenderPass.hpp"
#include "ShaderReflection.hpp"

#include <spirv_glsl.hpp>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::unordered_map<
  RenderPass const*,
  std::vector<std::pair<Material::PipelineCreateInfo, std::shared_ptr<vk::Pipeline>>>>
  Material::mPipelineCache;

////////////////////////////////////////////////////////////////////////////////////////////////////

Material::Material(
  std::shared_ptr<Engine> const&  engine,
  std::vector<std::string> const& shaderFiles,
  uint32_t                        materialCount)
  : mEngine(engine) {

  ILLUSION_TRACE << "Creating Material." << std::endl;

  mPipelineLayout = std::make_shared<PipelineLayout>(engine, shaderFiles, materialCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Material::~Material() { ILLUSION_TRACE << "Deleting Material." << std::endl; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Material::bind(
  vk::CommandBuffer const&                                cmd,
  std::shared_ptr<RenderPass> const&                      renderPass,
  uint32_t                                                subPass,
  vk::PrimitiveTopology                                   primitiveTopology,
  std::vector<vk::VertexInputBindingDescription> const&   inputBindings,
  std::vector<vk::VertexInputAttributeDescription> const& inputAttributes) const {

  PipelineCreateInfo info{
    this, renderPass.get(), subPass, primitiveTopology, inputBindings, inputAttributes};

  std::shared_ptr<vk::Pipeline> cachedPipeline = getCachedPipeline(info);

  if (!cachedPipeline) {
    cachedPipeline = createPipeline(info);
    mPipelineCache[info.mRenderPass].push_back(std::make_pair(info, cachedPipeline));
  }

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *cachedPipeline);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Pipeline> Material::getCachedPipeline(PipelineCreateInfo const& info) {

  for (auto const& p : mPipelineCache[info.mRenderPass]) {
    if (p.first == info) return p.second;
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Material::clearPipelineCache(RenderPass const* renderPass) {
  mPipelineCache.erase(renderPass);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Material::clearPipelineCache() { mPipelineCache.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<vk::Pipeline> Material::createPipeline(PipelineCreateInfo const& info) const {

  // vertex input ----------------------------------------------------------------------------------
  vk::PipelineVertexInputStateCreateInfo vertexInputState;
  vertexInputState.vertexBindingDescriptionCount   = info.mInputBindings.size();
  vertexInputState.pVertexBindingDescriptions      = info.mInputBindings.data();
  vertexInputState.vertexAttributeDescriptionCount = info.mInputAttributes.size();
  vertexInputState.pVertexAttributeDescriptions    = info.mInputAttributes.data();

  // input assembly --------------------------------------------------------------------------------
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
  inputAssemblyState.topology               = info.mPrimitiveTopology;
  inputAssemblyState.primitiveRestartEnable = false;

  // viewport state --------------------------------------------------------------------------------
  vk::PipelineViewportStateCreateInfo viewportState;
  viewportState.viewportCount = 1;
  viewportState.scissorCount  = 1;

  // dynamic state ---------------------------------------------------------------------------------
  std::vector<vk::DynamicState> dynamicStates;
  dynamicStates.push_back(vk::DynamicState::eViewport);
  dynamicStates.push_back(vk::DynamicState::eScissor);

  vk::PipelineDynamicStateCreateInfo dynamicState;
  dynamicState.pDynamicStates    = dynamicStates.data();
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

  // rasterizer ------------------------------------------------------------------------------------
  vk::PipelineRasterizationStateCreateInfo rasterizerState;
  rasterizerState.depthClampEnable        = false;
  rasterizerState.rasterizerDiscardEnable = false;
  rasterizerState.polygonMode             = vk::PolygonMode::eFill;
  rasterizerState.lineWidth               = 1.0f;
  rasterizerState.cullMode                = vk::CullModeFlagBits::eNone;
  rasterizerState.frontFace               = vk::FrontFace::eCounterClockwise;
  rasterizerState.depthBiasEnable         = false;

  // multisampling ---------------------------------------------------------------------------------
  vk::PipelineMultisampleStateCreateInfo multisamplingState;
  multisamplingState.sampleShadingEnable  = false;
  multisamplingState.rasterizationSamples = vk::SampleCountFlagBits::e1;

  // color blending --------------------------------------------------------------------------------
  vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
  colorBlendAttachmentState.colorWriteMask =
    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
    vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachmentState.blendEnable         = true;
  colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachmentState.colorBlendOp        = vk::BlendOp::eAdd;
  colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
  colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
  colorBlendAttachmentState.alphaBlendOp        = vk::BlendOp::eAdd;

  vk::PipelineColorBlendStateCreateInfo colorBlendState;
  colorBlendState.logicOpEnable   = false;
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments    = &colorBlendAttachmentState;

  // depth stencil state ---------------------------------------------------------------------------
  vk::PipelineDepthStencilStateCreateInfo depthStencilState;
  depthStencilState.depthWriteEnable  = true;
  depthStencilState.depthTestEnable   = true;
  depthStencilState.stencilTestEnable = false;
  depthStencilState.depthCompareOp    = vk::CompareOp::eLess;
  depthStencilState.minDepthBounds    = 0;
  depthStencilState.maxDepthBounds    = 1;

  // shader state ----------------------------------------------------------------------------------
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  std::vector<std::shared_ptr<vk::ShaderModule>> shaderModules;

  for (size_t i{0}; i < mPipelineLayout->getStageReflections().size(); ++i) {
    vk::ShaderModuleCreateInfo moduleInfo;
    moduleInfo.codeSize = mPipelineLayout->getShaderCodes()[i].size() * 4;
    moduleInfo.pCode    = mPipelineLayout->getShaderCodes()[i].data();

    auto module = mEngine->createShaderModule(moduleInfo);

    vk::PipelineShaderStageCreateInfo stageInfo;
    stageInfo.stage = vk::ShaderStageFlagBits(
      (VkShaderStageFlags)mPipelineLayout->getStageReflections()[i]->getStages());
    stageInfo.module = *module;
    stageInfo.pName  = "main";

    shaderStages.push_back(stageInfo);

    // prevent destruction in this scope
    shaderModules.push_back(module);
  }

  // create pipeline -------------------------------------------------------------------------------
  vk::GraphicsPipelineCreateInfo createInfo;
  createInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
  createInfo.pStages             = shaderStages.data();
  createInfo.pVertexInputState   = &vertexInputState;
  createInfo.pInputAssemblyState = &inputAssemblyState;
  createInfo.pViewportState      = &viewportState;
  createInfo.pRasterizationState = &rasterizerState;
  createInfo.pMultisampleState   = &multisamplingState;
  createInfo.pColorBlendState    = &colorBlendState;
  createInfo.pDepthStencilState  = &depthStencilState;
  createInfo.pDynamicState       = &dynamicState;
  createInfo.layout              = *mPipelineLayout->getLayout();
  createInfo.subpass             = info.mSubPass;
  createInfo.basePipelineHandle  = nullptr;

  return info.mRenderPass->createPipeline(createInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Illusion::Graphics
