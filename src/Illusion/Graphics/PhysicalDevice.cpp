////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PhysicalDevice.hpp"

#include "../Core/EnumCast.hpp"
#include "../Core/Logger.hpp"

#include <GLFW/glfw3.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Illusion::Graphics {

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

void printCap(std::string const& name, vk::Bool32 cap) {
  ILLUSION_MESSAGE << std::left << std::setw(50) << std::setfill('.') << (name + " ")
                   << (cap ? Core::Logger::PRINT_GREEN + " yes" : Core::Logger::PRINT_RED + " no")
                   << Core::Logger::PRINT_RESET << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void printVal(std::string const& name, std::vector<std::string> const& vals) {
  std::stringstream sstr;

  for (size_t i(0); i < vals.size(); ++i) {
    sstr << vals[i];
    if (i < vals.size() - 1) {
      sstr << " | ";
    }
  }

  ILLUSION_MESSAGE << std::left << std::setw(50) << std::setfill('.') << (name + " ") << " "
                   << sstr.str() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename S, typename T>
std::string printMin(S val, T ref) {
  std::string color{Core::Logger::PRINT_RED};

  if (val == ref)
    color = Core::Logger::PRINT_YELLOW;
  else if (val > ref)
    color = Core::Logger::PRINT_GREEN;

  return color + std::to_string(val) + Core::Logger::PRINT_RESET + " (" + std::to_string(ref) + ")";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename S, typename T>
std::string printMax(S val, T ref) {
  std::string color{Core::Logger::PRINT_RED};

  if (val == ref)
    color = Core::Logger::PRINT_YELLOW;
  else if (val < ref)
    color = Core::Logger::PRINT_GREEN;

  return color + std::to_string(val) + Core::Logger::PRINT_RESET + " (" + std::to_string(ref) + ")";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

PhysicalDevice::PhysicalDevice(vk::Instance const& instance, vk::PhysicalDevice const& device)
    : vk::PhysicalDevice(device) {

  auto                available  = getQueueFamilyProperties();
  std::array<bool, 3> foundQueue = {false, false, false};

  // first find a family which can do everything
  for (size_t i(0); i < available.size(); ++i) {
    vk::QueueFlags required(
        vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer);

    if (available[i].queueCount > 0 && (available[i].queueFlags & required) == required &&
        glfwGetPhysicalDevicePresentationSupport(instance, *this, static_cast<uint32_t>(i))) {

      mQueueFamilies[Core::enumCast(QueueType::eGeneric)] = static_cast<uint32_t>(i);
      foundQueue[Core::enumCast(QueueType::eGeneric)]     = true;
      break;
    }
  }

  // then find a different family for compute
  for (size_t i(0); i < available.size(); ++i) {
    vk::QueueFlags required(vk::QueueFlagBits::eCompute);

    if (available[i].queueCount > 0 && (available[i].queueFlags & required) == required &&
        i != mQueueFamilies[Core::enumCast(QueueType::eGeneric)]) {

      mQueueFamilies[Core::enumCast(QueueType::eCompute)] = static_cast<uint32_t>(i);
      foundQueue[Core::enumCast(QueueType::eCompute)]     = true;
      break;
    }
  }

  // then find a different family for transfer
  for (size_t i(0); i < available.size(); ++i) {
    vk::QueueFlags required(vk::QueueFlagBits::eTransfer);

    if (available[i].queueCount > 0 && (available[i].queueFlags & required) == required &&
        i != mQueueFamilies[Core::enumCast(QueueType::eGeneric)] &&
        i != mQueueFamilies[Core::enumCast(QueueType::eCompute)]) {

      mQueueFamilies[Core::enumCast(QueueType::eTransfer)] = static_cast<uint32_t>(i);
      foundQueue[Core::enumCast(QueueType::eTransfer)]     = true;
      break;
    }
  }

  // if we did not find a transfer family which is different from compute and generic, we might use
  // the same as for compute
  for (size_t i(0); i < available.size(); ++i) {
    vk::QueueFlags required(vk::QueueFlagBits::eTransfer);

    if (available[i].queueCount > 0 && (available[i].queueFlags & required) == required &&
        i != mQueueFamilies[Core::enumCast(QueueType::eGeneric)]) {

      mQueueFamilies[Core::enumCast(QueueType::eTransfer)] = static_cast<uint32_t>(i);
      foundQueue[Core::enumCast(QueueType::eTransfer)]     = true;
      break;
    }
  }

  // if we did not find a compute queue different from the generic one, we will use the same but
  // another index, if possible
  if (!foundQueue[Core::enumCast(QueueType::eCompute)]) {

    mQueueFamilies[Core::enumCast(QueueType::eCompute)] =
        mQueueFamilies[Core::enumCast(QueueType::eGeneric)];
    mQueueIndices[Core::enumCast(QueueType::eCompute)] =
        std::min(available[mQueueFamilies[Core::enumCast(QueueType::eCompute)]].queueCount - 1,
            mQueueIndices[Core::enumCast(QueueType::eGeneric)] + 1);
  }

  // if we did not find a transfer queue different from the generic one, we will use the same but
  // another index, if possible
  if (!foundQueue[Core::enumCast(QueueType::eTransfer)]) {
    mQueueFamilies[Core::enumCast(QueueType::eTransfer)] =
        mQueueFamilies[Core::enumCast(QueueType::eGeneric)];
    mQueueIndices[Core::enumCast(QueueType::eTransfer)] =
        std::min(available[mQueueFamilies[Core::enumCast(QueueType::eTransfer)]].queueCount - 1,
            mQueueIndices[Core::enumCast(QueueType::eCompute)] + 1);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t PhysicalDevice::findMemoryType(
    uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {

  auto memProperties{getMemoryProperties()};

  for (uint32_t i{0}; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error{"Failed to find suitable memory type."};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t PhysicalDevice::getQueueFamily(QueueType type) const {
  return mQueueFamilies[Core::enumCast(type)];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t PhysicalDevice::getQueueIndex(QueueType type) const {
  return mQueueFamilies[Core::enumCast(type)];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::printInfo() {
  // basic information
  vk::PhysicalDeviceProperties properties{getProperties()};
  ILLUSION_MESSAGE << Core::Logger::PRINT_BOLD << "Physical Device Information "
                   << Core::Logger::PRINT_RESET << std::endl;
  printVal("apiVersion", {std::to_string(properties.apiVersion)});
  printVal("driverVersion", {std::to_string(properties.driverVersion)});
  printVal("vendorID", {std::to_string(properties.vendorID)});
  printVal("deviceID", {std::to_string(properties.deviceID)});
  printVal("deviceType", {vk::to_string(properties.deviceType)});
  printVal("deviceName", {properties.deviceName});

  // memory information
  vk::PhysicalDeviceMemoryProperties memoryProperties = getMemoryProperties();
  ILLUSION_MESSAGE << Core::Logger::PRINT_BOLD << "Memory Information " << Core::Logger::PRINT_RESET
                   << std::endl;
  for (unsigned i{0}; i < memoryProperties.memoryTypeCount; ++i) {
    printVal("Memory type " + std::to_string(i),
        {vk::to_string(memoryProperties.memoryTypes[i].propertyFlags)});
  }

  for (unsigned i{0}; i < memoryProperties.memoryHeapCount; ++i) {
    printVal("Memory heap " + std::to_string(i),
        {std::to_string(memoryProperties.memoryHeaps[i].size / (1024 * 1024)) + " MB " +
            vk::to_string(memoryProperties.memoryHeaps[i].flags)});
  }

  // clang-format off
  // features
  vk::PhysicalDeviceFeatures features{getFeatures()};
  ILLUSION_MESSAGE << Core::Logger::PRINT_BOLD << "Features " << Core::Logger::PRINT_RESET << std::endl;
  printCap("robustBufferAccess",                      features.robustBufferAccess);
  printCap("fullDrawIndexUint32",                     features.fullDrawIndexUint32);
  printCap("imageCubeArray",                          features.imageCubeArray);
  printCap("independentBlend",                        features.independentBlend);
  printCap("geometryShader",                          features.geometryShader);
  printCap("tessellationShader",                      features.tessellationShader);
  printCap("sampleRateShading",                       features.sampleRateShading);
  printCap("dualSrcBlend",                            features.dualSrcBlend);
  printCap("logicOp",                                 features.logicOp);
  printCap("multiDrawIndirect",                       features.multiDrawIndirect);
  printCap("drawIndirectFirstInstance",               features.drawIndirectFirstInstance);
  printCap("depthClamp",                              features.depthClamp);
  printCap("depthBiasClamp",                          features.depthBiasClamp);
  printCap("fillModeNonSolid",                        features.fillModeNonSolid);
  printCap("depthBounds",                             features.depthBounds);
  printCap("wideLines",                               features.wideLines);
  printCap("largePoints",                             features.largePoints);
  printCap("alphaToOne",                              features.alphaToOne);
  printCap("multiViewport",                           features.multiViewport);
  printCap("samplerAnisotropy",                       features.samplerAnisotropy);
  printCap("textureCompressionETC2",                  features.textureCompressionETC2);
  printCap("textureCompressionASTC_LDR",              features.textureCompressionASTC_LDR);
  printCap("textureCompressionBC",                    features.textureCompressionBC);
  printCap("occlusionQueryPrecise",                   features.occlusionQueryPrecise);
  printCap("pipelineStatisticsQuery",                 features.pipelineStatisticsQuery);
  printCap("vertexPipelineStoresAndAtomics",          features.vertexPipelineStoresAndAtomics);
  printCap("fragmentStoresAndAtomics",                features.fragmentStoresAndAtomics);
  printCap("shaderTessellationAndGeometryPointSize",  features.shaderTessellationAndGeometryPointSize);
  printCap("shaderImageGatherExtended",               features.shaderImageGatherExtended);
  printCap("shaderStorageImageExtendedFormats",       features.shaderStorageImageExtendedFormats);
  printCap("shaderStorageImageMultisample",           features.shaderStorageImageMultisample);
  printCap("shaderStorageImageReadWithoutFormat",     features.shaderStorageImageReadWithoutFormat);
  printCap("shaderStorageImageWriteWithoutFormat",    features.shaderStorageImageWriteWithoutFormat);
  printCap("shaderUniformBufferArrayDynamicIndexing", features.shaderUniformBufferArrayDynamicIndexing);
  printCap("shaderSampledImageArrayDynamicIndexing",  features.shaderSampledImageArrayDynamicIndexing);
  printCap("shaderStorageBufferArrayDynamicIndexing", features.shaderStorageBufferArrayDynamicIndexing);
  printCap("shaderStorageImageArrayDynamicIndexing",  features.shaderStorageImageArrayDynamicIndexing);
  printCap("shaderClipDistance",                      features.shaderClipDistance);
  printCap("shaderCullDistance",                      features.shaderCullDistance);
  printCap("shaderFloat64",                           features.shaderFloat64);
  printCap("shaderInt64",                             features.shaderInt64);
  printCap("shaderInt16",                             features.shaderInt16);
  printCap("shaderResourceResidency",                 features.shaderResourceResidency);
  printCap("shaderResourceMinLod",                    features.shaderResourceMinLod);
  printCap("sparseBinding",                           features.sparseBinding);
  printCap("sparseResidencyBuffer",                   features.sparseResidencyBuffer);
  printCap("sparseResidencyImage2D",                  features.sparseResidencyImage2D);
  printCap("sparseResidencyImage3D",                  features.sparseResidencyImage3D);
  printCap("sparseResidency2Samples",                 features.sparseResidency2Samples);
  printCap("sparseResidency4Samples",                 features.sparseResidency4Samples);
  printCap("sparseResidency8Samples",                 features.sparseResidency8Samples);
  printCap("sparseResidency16Samples",                features.sparseResidency16Samples);
  printCap("sparseResidencyAliased",                  features.sparseResidencyAliased);
  printCap("variableMultisampleRate",                 features.variableMultisampleRate);
  printCap("inheritedQueries",                        features.inheritedQueries);

  // format properties
  ILLUSION_MESSAGE << Core::Logger::PRINT_BOLD << "Format Properties " << Core::Logger::PRINT_RESET << std::endl;
  for (int i(VK_FORMAT_BEGIN_RANGE+1); i<=VK_FORMAT_END_RANGE; ++i)
  {
    vk::FormatProperties props = getFormatProperties(vk::Format(i));
    std::stringstream sstr;

    if (props.optimalTilingFeatures || props.linearTilingFeatures || props.bufferFeatures) {
      sstr << Core::Logger::PRINT_GREEN + "yes" << Core::Logger::PRINT_RESET;
    } else {
      sstr << Core::Logger::PRINT_RED + "no" << Core::Logger::PRINT_RESET;
    }

    ILLUSION_MESSAGE << std::left << std::setw(50) << std::setfill('.') 
                     << (vk::to_string(vk::Format(i)) + " ") << " "
                     << sstr.str() << std::endl;

    if (props.optimalTilingFeatures) {
      ILLUSION_MESSAGE << std::right << std::setw(50) << std::setfill(' ') 
                       << "Optimal Tiling:" << " " << vk::to_string(props.optimalTilingFeatures) 
                       << std::endl;
    }

    if (props.linearTilingFeatures) {
      ILLUSION_MESSAGE << std::right << std::setw(50) << std::setfill(' ') 
                       << "Linear Tiling:" << " " << vk::to_string(props.linearTilingFeatures) 
                       << std::endl;
    }

    if (props.bufferFeatures) {
      ILLUSION_MESSAGE << std::right << std::setw(50) << std::setfill(' ') 
                       << "Buffer Features:" << " " << vk::to_string(props.bufferFeatures) 
                       << std::endl;
    }

  }

  // limits
  vk::PhysicalDeviceLimits limits = properties.limits;
  ILLUSION_MESSAGE << Core::Logger::PRINT_BOLD << "Limits " << Core::Logger::PRINT_RESET << std::endl;
  printVal("maxImageDimension1D",                             {printMin(limits.maxImageDimension1D, 4096u)});
  printVal("maxImageDimension2D",                             {printMin(limits.maxImageDimension2D, 4096u)});
  printVal("maxImageDimension3D",                             {printMin(limits.maxImageDimension3D, 256u)});
  printVal("maxImageDimensionCube",                           {printMin(limits.maxImageDimensionCube, 4096u)});
  printVal("maxImageArrayLayers",                             {printMin(limits.maxImageArrayLayers, 256u)});
  printVal("maxTexelBufferElements",                          {printMin(limits.maxTexelBufferElements, 65536u)});
  printVal("maxUniformBufferRange",                           {printMin(limits.maxUniformBufferRange, 16384u)});
  printVal("maxStorageBufferRange",                           {printMin(limits.maxStorageBufferRange, (unsigned)std::pow(2, 27))});
  printVal("maxPushConstantsSize",                            {printMin(limits.maxPushConstantsSize, 128u)});
  printVal("maxMemoryAllocationCount",                        {printMin(limits.maxMemoryAllocationCount, 4096u)});
  printVal("maxSamplerAllocationCount",                       {printMin(limits.maxSamplerAllocationCount, 4000u)});
  printVal("bufferImageGranularity",                          {printMax(limits.bufferImageGranularity, 131072u)});
  printVal("sparseAddressSpaceSize",                          {printMin(limits.sparseAddressSpaceSize, (unsigned)std::pow(2, 31))});
  printVal("maxBoundDescriptorSets",                          {printMin(limits.maxBoundDescriptorSets, 4u)});
  printVal("maxPerStageDescriptorSamplers",                   {printMin(limits.maxPerStageDescriptorSamplers, 16u)});
  printVal("maxPerStageDescriptorUniformBuffers",             {printMin(limits.maxPerStageDescriptorUniformBuffers, 12u)});
  printVal("maxPerStageDescriptorStorageBuffers",             {printMin(limits.maxPerStageDescriptorStorageBuffers, 4u)});
  printVal("maxPerStageDescriptorSampledImages",              {printMin(limits.maxPerStageDescriptorSampledImages, 16u)});
  printVal("maxPerStageDescriptorStorageImages",              {printMin(limits.maxPerStageDescriptorStorageImages, 4u)});
  printVal("maxPerStageDescriptorInputAttachments",           {printMin(limits.maxPerStageDescriptorInputAttachments, 4u)});
  printVal("maxPerStageResources",                            {printMin(limits.maxPerStageResources, 128u)});
  printVal("maxDescriptorSetSamplers",                        {printMin(limits.maxDescriptorSetSamplers, 96u)});
  printVal("maxDescriptorSetUniformBuffers",                  {printMin(limits.maxDescriptorSetUniformBuffers, 72u)});
  printVal("maxDescriptorSetUniformBuffersDynamic",           {printMin(limits.maxDescriptorSetUniformBuffersDynamic, 8u)});
  printVal("maxDescriptorSetStorageBuffers",                  {printMin(limits.maxDescriptorSetStorageBuffers, 24u)});
  printVal("maxDescriptorSetStorageBuffersDynamic",           {printMin(limits.maxDescriptorSetStorageBuffersDynamic, 4u)});
  printVal("maxDescriptorSetSampledImages",                   {printMin(limits.maxDescriptorSetSampledImages, 96u)});
  printVal("maxDescriptorSetStorageImages",                   {printMin(limits.maxDescriptorSetStorageImages, 24u)});
  printVal("maxDescriptorSetInputAttachments",                {printMin(limits.maxDescriptorSetInputAttachments, 4u)});
  printVal("maxVertexInputAttributes",                        {printMin(limits.maxVertexInputAttributes, 16u)});
  printVal("maxVertexInputBindings",                          {printMin(limits.maxVertexInputBindings, 16u)});
  printVal("maxVertexInputAttributeOffset",                   {printMin(limits.maxVertexInputAttributeOffset, 2047u)});
  printVal("maxVertexInputBindingStride",                     {printMin(limits.maxVertexInputBindingStride, 2048u)});
  printVal("maxVertexOutputComponents",                       {printMin(limits.maxVertexOutputComponents, 64u)});
  printVal("maxTessellationGenerationLevel",                  {printMin(limits.maxTessellationGenerationLevel, 64u)});
  printVal("maxTessellationPatchSize",                        {printMin(limits.maxTessellationPatchSize, 32u)});
  printVal("maxTessellationControlPerVertexInputComponents",  {printMin(limits.maxTessellationControlPerVertexInputComponents, 64u)});
  printVal("maxTessellationControlPerVertexOutputComponents", {printMin(limits.maxTessellationControlPerVertexOutputComponents, 64u)});
  printVal("maxTessellationControlPerPatchOutputComponents",  {printMin(limits.maxTessellationControlPerPatchOutputComponents, 120u)});
  printVal("maxTessellationControlTotalOutputComponents",     {printMin(limits.maxTessellationControlTotalOutputComponents, 2048u)});
  printVal("maxTessellationEvaluationInputComponents",        {printMin(limits.maxTessellationEvaluationInputComponents, 64u)});
  printVal("maxTessellationEvaluationOutputComponents",       {printMin(limits.maxTessellationEvaluationOutputComponents, 64u)});
  printVal("maxGeometryShaderInvocations",                    {printMin(limits.maxGeometryShaderInvocations, 32u)});
  printVal("maxGeometryInputComponents",                      {printMin(limits.maxGeometryInputComponents, 64u)});
  printVal("maxGeometryOutputComponents",                     {printMin(limits.maxGeometryOutputComponents, 64u)});
  printVal("maxGeometryOutputVertices",                       {printMin(limits.maxGeometryOutputVertices, 256u)});
  printVal("maxGeometryTotalOutputComponents",                {printMin(limits.maxGeometryTotalOutputComponents, 1024u)});
  printVal("maxFragmentInputComponents",                      {printMin(limits.maxFragmentInputComponents, 64u)});
  printVal("maxFragmentOutputAttachments",                    {printMin(limits.maxFragmentOutputAttachments, 4u)});
  printVal("maxFragmentDualSrcAttachments",                   {printMin(limits.maxFragmentDualSrcAttachments, 1u)});
  printVal("maxFragmentCombinedOutputResources",              {printMin(limits.maxFragmentCombinedOutputResources, 4u)});
  printVal("maxComputeSharedMemorySize",                      {printMin(limits.maxComputeSharedMemorySize, 16384u)});
  printVal("maxComputeWorkGroupCount",                        {printMin(limits.maxComputeWorkGroupCount[0], 65535u), printMin(limits.maxComputeWorkGroupCount[1], 65535u), printMin(limits.maxComputeWorkGroupCount[2], 65535u)});
  printVal("maxComputeWorkGroupInvocations",                  {printMin(limits.maxComputeWorkGroupInvocations, 128u)});
  printVal("maxComputeWorkGroupSize",                         {printMin(limits.maxComputeWorkGroupSize[0], 128u), printMin(limits.maxComputeWorkGroupSize[1], 128u), printMin(limits.maxComputeWorkGroupSize[2], 64u)});
  printVal("subPixelPrecisionBits",                           {printMin(limits.subPixelPrecisionBits, 4u)});
  printVal("subTexelPrecisionBits",                           {printMin(limits.subTexelPrecisionBits, 4u)});
  printVal("mipmapPrecisionBits",                             {printMin(limits.mipmapPrecisionBits, 4u)});
  printVal("maxDrawIndexedIndexValue",                        {printMin(limits.maxDrawIndexedIndexValue, (unsigned)(std::pow(2, 32) - 1))});
  printVal("maxDrawIndirectCount",                            {printMin(limits.maxDrawIndirectCount, (unsigned)(std::pow(2, 16) - 1))});
  printVal("maxSamplerLodBias",                               {printMin(limits.maxSamplerLodBias, 2)});
  printVal("maxSamplerAnisotropy",                            {printMin(limits.maxSamplerAnisotropy, 16)});
  printVal("maxViewports",                                    {printMin(limits.maxViewports, 16u)});
  printVal("maxViewportDimensions",                           {printMin(limits.maxViewportDimensions[0], 4096u), printMin(limits.maxViewportDimensions[1], 4096u)});
  printVal("viewportBoundsRange",                             {printMax(limits.viewportBoundsRange[0], -8192), printMin(limits.viewportBoundsRange[1], 8191)});
  printVal("viewportSubPixelBits",                            {printMin(limits.viewportSubPixelBits, 0u)});
  printVal("minMemoryMapAlignment",                           {printMin(limits.minMemoryMapAlignment, 64u)});
  printVal("minTexelBufferOffsetAlignment",                   {printMax(limits.minTexelBufferOffsetAlignment, 256u)});
  printVal("minUniformBufferOffsetAlignment",                 {printMax(limits.minUniformBufferOffsetAlignment, 256u)});
  printVal("minStorageBufferOffsetAlignment",                 {printMax(limits.minStorageBufferOffsetAlignment, 256u)});
  printVal("minTexelOffset",                                  {printMax(limits.minTexelOffset, -8)});
  printVal("maxTexelOffset",                                  {printMin(limits.maxTexelOffset, 7u)});
  printVal("minTexelGatherOffset",                            {printMax(limits.minTexelGatherOffset, -8)});
  printVal("maxTexelGatherOffset",                            {printMin(limits.maxTexelGatherOffset, 7u)});
  printVal("minInterpolationOffset",                          {printMax(limits.minInterpolationOffset, 0.5)});
  printVal("maxInterpolationOffset",                          {printMin(limits.maxInterpolationOffset, 0.5 - std::pow(0.5, limits.subPixelInterpolationOffsetBits))});
  printVal("subPixelInterpolationOffsetBits",                 {printMin(limits.subPixelInterpolationOffsetBits, 4u)});
  printVal("maxFramebufferWidth",                             {printMin(limits.maxFramebufferWidth, 4096u)});
  printVal("maxFramebufferHeight",                            {printMin(limits.maxFramebufferHeight, 4096u)});
  printVal("maxFramebufferLayers",                            {printMin(limits.maxFramebufferLayers, 256u)});
  printVal("framebufferColorSampleCounts",                    {vk::to_string(limits.framebufferColorSampleCounts) + " ({1 | 4})"});
  printVal("framebufferDepthSampleCounts",                    {vk::to_string(limits.framebufferDepthSampleCounts) + " ({1 | 4})"});
  printVal("framebufferStencilSampleCounts",                  {vk::to_string(limits.framebufferStencilSampleCounts) + " ({1 | 4})"});
  printVal("framebufferNoAttachmentsSampleCounts",            {vk::to_string(limits.framebufferNoAttachmentsSampleCounts) + " ({1 | 4})"});
  printVal("maxColorAttachments",                             {printMin(limits.maxColorAttachments, 4u)});
  printVal("sampledImageColorSampleCounts",                   {vk::to_string(limits.sampledImageColorSampleCounts) + " ({1 | 4})"});
  printVal("sampledImageIntegerSampleCounts",                 {vk::to_string(limits.sampledImageIntegerSampleCounts) + " ({1})"});
  printVal("sampledImageDepthSampleCounts",                   {vk::to_string(limits.sampledImageDepthSampleCounts) + " ({1 | 4})"});
  printVal("sampledImageStencilSampleCounts",                 {vk::to_string(limits.sampledImageStencilSampleCounts) + " ({1 | 4})"});
  printVal("storageImageSampleCounts",                        {vk::to_string(limits.storageImageSampleCounts) + " ({1 | 4})"});
  printVal("maxSampleMaskWords",                              {printMin(limits.maxSampleMaskWords, 1u)});
  printVal("timestampComputeAndGraphics",                     {std::to_string(limits.timestampComputeAndGraphics)});
  printVal("timestampPeriod",                                 {std::to_string(limits.timestampPeriod)});
  printVal("maxClipDistances",                                {printMin(limits.maxClipDistances, 8u)});
  printVal("maxCullDistances",                                {printMin(limits.maxCullDistances, 8u)});
  printVal("maxCombinedClipAndCullDistances",                 {printMin(limits.maxCombinedClipAndCullDistances, 8u)});
  printVal("discreteQueuePriorities",                         {printMin(limits.discreteQueuePriorities, 2u)});
  printVal("pointSizeRange",                                  {printMax(limits.pointSizeRange[0], 1.0), printMin(limits.pointSizeRange[1], 64.0 - limits.pointSizeGranularity)});
  printVal("lineWidthRange",                                  {printMax(limits.lineWidthRange[0], 1.0), printMin(limits.lineWidthRange[1], 8.0 - limits.lineWidthGranularity)});
  printVal("pointSizeGranularity",                            {printMax(limits.pointSizeGranularity, 1)});
  printVal("lineWidthGranularity",                            {printMax(limits.lineWidthGranularity, 1)});
  printVal("strictLines",                                     {std::to_string(limits.strictLines)});
  printVal("standardSampleLocations",                         {std::to_string(limits.standardSampleLocations)});
  printVal("optimalBufferCopyOffsetAlignment",                {std::to_string(limits.optimalBufferCopyOffsetAlignment)});
  printVal("optimalBufferCopyRowPitchAlignment",              {std::to_string(limits.optimalBufferCopyRowPitchAlignment)});
  printVal("nonCoherentAtomSize",                             {printMax(limits.nonCoherentAtomSize, 256u)});

  // clang-format on
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
