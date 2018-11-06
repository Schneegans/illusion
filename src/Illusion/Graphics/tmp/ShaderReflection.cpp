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
#include "ShaderReflection.hpp"

#include <iomanip>
#include <spirv_cpp.hpp>
#include <spirv_glsl.hpp>

namespace Illusion::Graphics {
namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

class Parser : public spirv_cross::CompilerGLSL {
 public:
  Parser(std::vector<uint32_t> const& ir)
    : spirv_cross::CompilerGLSL(ir) {}

  uint32_t getAlignment(
    spirv_cross::SPIRType const&              type,
    ShaderReflection::Buffer::PackingStandard packing,
    uint32_t                                  flags) {

    spirv_cross::BufferPackingStandard p;
    if (packing == ShaderReflection::Buffer::PackingStandard::eStd430)
      p = spirv_cross::BufferPackingStd430;
    else
      p = spirv_cross::BufferPackingStd140;

    return spirv_cross::CompilerGLSL::type_to_packed_alignment(type, flags, p);
  }

  ShaderReflection::Buffer::PackingStandard getPackingStandard(spirv_cross::SPIRType const& type) {
    if (buffer_is_packing_standard(type, spirv_cross::BufferPackingStd140))
      return ShaderReflection::Buffer::PackingStandard::eStd140;
    if (buffer_is_packing_standard(type, spirv_cross::BufferPackingStd430))
      return ShaderReflection::Buffer::PackingStandard::eStd430;

    throw std::runtime_error{"Invalid spirv_cross::BufferPackingStandard type!"};
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string stagesToInfoString(vk::ShaderStageFlags stages) {
  std::string result;

  auto addStage = [&](std::string const& name, vk::ShaderStageFlags stage) {
    if ((VkShaderStageFlags)(stages & stage) > 0) {
      if (result.size() > 0) { result += " | "; }
      result += name;
    }
  };

  addStage("Compute", vk::ShaderStageFlagBits::eCompute);
  addStage("Fragment", vk::ShaderStageFlagBits::eFragment);
  addStage("Geometry", vk::ShaderStageFlagBits::eGeometry);
  addStage("TessellationControl", vk::ShaderStageFlagBits::eTessellationControl);
  addStage("TessellationEvaluation", vk::ShaderStageFlagBits::eTessellationEvaluation);
  addStage("Vertex", vk::ShaderStageFlagBits::eVertex);

  if (result.size() == 0) { result = "None"; }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string stagesToCppString(vk::ShaderStageFlags stages) {
  std::string result;

  auto addStage = [&](std::string const& name, vk::ShaderStageFlags stage) {
    if ((VkShaderStageFlags)(stages & stage) > 0) {
      if (result.size() > 0) { result += " | "; }
      result += name;
    }
  };

  addStage("vk::ShaderStageFlagBits::eCompute", vk::ShaderStageFlagBits::eCompute);
  addStage("vk::ShaderStageFlagBits::eFragment", vk::ShaderStageFlagBits::eFragment);
  addStage("vk::ShaderStageFlagBits::eGeometry", vk::ShaderStageFlagBits::eGeometry);
  addStage(
    "vk::ShaderStageFlagBits::eTessellationControl", vk::ShaderStageFlagBits::eTessellationControl);
  addStage(
    "vk::ShaderStageFlagBits::eTessellationEvaluation",
    vk::ShaderStageFlagBits::eTessellationEvaluation);
  addStage("vk::ShaderStageFlagBits::eVertex", vk::ShaderStageFlagBits::eVertex);

  if (result.size() == 0) { result = "vk::ShaderStageFlags()"; }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::BufferRange::BaseType convert(spirv_cross::SPIRType type) {
  switch (type.basetype) {
  case spirv_cross::SPIRType::Int:
    return ShaderReflection::BufferRange::BaseType::eInt;
  case spirv_cross::SPIRType::Boolean:
  case spirv_cross::SPIRType::UInt:
    return ShaderReflection::BufferRange::BaseType::eUInt;
  case spirv_cross::SPIRType::Float:
    return ShaderReflection::BufferRange::BaseType::eFloat;
  case spirv_cross::SPIRType::Double:
    return ShaderReflection::BufferRange::BaseType::eDouble;
  case spirv_cross::SPIRType::Struct:
    return ShaderReflection::BufferRange::BaseType::eStruct;
  default:
    return ShaderReflection::BufferRange::BaseType::eUnknown;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::vector<uint32_t> const& code) {
  Parser                       parser{code};
  spirv_cross::ShaderResources resources       = parser.get_shader_resources();
  auto                         activeVariables = parser.get_active_interface_variables();

  // collect basic information ---------------------------------------------------------------------
  auto stage = parser.get_execution_model();

  switch (stage) {
  case spv::ExecutionModelVertex:
    mStages = vk::ShaderStageFlagBits::eVertex;
    break;
  case spv::ExecutionModelFragment:
    mStages = vk::ShaderStageFlagBits::eFragment;
    break;
  case spv::ExecutionModelGLCompute:
    mStages = vk::ShaderStageFlagBits::eCompute;
    break;
  default:
    throw std::runtime_error{"Shader stage is not supported!"};
    break;
  }

  // collect buffers -------------------------------------------------------------------------------
  std::function<std::vector<BufferRange>(
    uint32_t, std::vector<spirv_cross::BufferRange> const&, Buffer::PackingStandard)>
    getBufferRanges = [&](
                        uint32_t                                     typeID,
                        std::vector<spirv_cross::BufferRange> const& activeRanges,
                        Buffer::PackingStandard                      packing) {
      std::vector<BufferRange> ranges;

      auto type = parser.get_type(typeID);

      for (size_t i{0}; i < type.member_types.size(); ++i) {
        BufferRange range;
        auto        memberType = parser.get_type(type.member_types[i]);
        uint32_t    flags      = parser.get_member_decoration_mask(type.self, i);

        range.mName      = parser.get_member_name(type.self, i);
        range.mSize      = parser.get_declared_struct_member_size(type, i);
        range.mOffset    = parser.type_struct_member_offset(type, i);
        range.mAlignment = parser.getAlignment(memberType, packing, flags);

        for (auto const& activeRange : activeRanges) {
          if (activeRange.index == i) { range.mActiveStages = mStages; }
        }

        range.mBaseType = convert(memberType);
        range.mBaseSize = memberType.width / 8;

        // vector types
        range.mElements = memberType.vecsize;

        // matrix types
        if (parser.has_member_decoration(type.self, i, spv::Decoration::DecorationMatrixStride)) {
          range.mColumns      = memberType.columns;
          range.mRows         = memberType.vecsize;
          range.mMatrixStride = parser.type_struct_member_matrix_stride(type, i);
        }

        // array types
        if (!memberType.array.empty()) {
          range.mArrayLengths = memberType.array;
          range.mArrayStride  = parser.type_struct_member_array_stride(type, i);
        }

        // struct types
        if (range.mBaseType == BufferRange::BaseType::eStruct) {
          range.mTypeName = parser.get_name(memberType.self);
          range.mMembers  = getBufferRanges(type.member_types[i], {}, packing);
        }

        ranges.push_back(range);
      }

      return ranges;
    };

  auto getBuffers = [&](std::vector<spirv_cross::Resource> const& resources) {
    std::vector<Buffer> result;

    for (auto const& resource : resources) {
      Buffer buffer;
      auto   type = parser.get_type(resource.type_id);

      buffer.mName            = parser.get_name(resource.id);
      buffer.mType            = parser.get_name(resource.base_type_id);
      buffer.mSize            = parser.get_declared_struct_size(type);
      buffer.mBinding         = parser.get_decoration(resource.id, spv::DecorationBinding);
      buffer.mSet             = parser.get_decoration(resource.id, spv::DecorationDescriptorSet);
      buffer.mPackingStandard = parser.getPackingStandard(type);

      mActiveDescriptorSets.insert(buffer.mSet);

      if (activeVariables.find(resource.id) != activeVariables.end()) {
        buffer.mActiveStages = mStages;
      }

      auto activeMembers{parser.get_active_buffer_ranges(resource.id)};

      buffer.mRanges = getBufferRanges(resource.type_id, activeMembers, buffer.mPackingStandard);

      result.push_back(buffer);
    }

    return result;
  };

  mPushConstantBuffers = getBuffers(resources.push_constant_buffers);
  mUniformBuffers      = getBuffers(resources.uniform_buffers);

  // collect image samplers ------------------------------------------------------------------------
  auto getSamplers = [&](std::vector<spirv_cross::Resource> const& resources) {
    std::vector<Sampler> result;
    for (auto const& resource : resources) {
      Sampler sampler;
      sampler.mName    = resource.name;
      sampler.mBinding = parser.get_decoration(resource.id, spv::DecorationBinding);
      sampler.mSet     = parser.get_decoration(resource.id, spv::DecorationDescriptorSet);

      mActiveDescriptorSets.insert(sampler.mSet);

      if (activeVariables.find(resource.id) != activeVariables.end()) {
        sampler.mActiveStages = mStages;
      }

      result.push_back(sampler);
    }
    return result;
  };

  mSamplers      = getSamplers(resources.sampled_images);
  mStorageImages = getSamplers(resources.storage_images);

  // warn if not supported features are used -------------------------------------------------------
  auto errorNotSupported =
    [](std::string const& name, std::vector<spirv_cross::Resource> const& resources) {
      if (resources.size() > 0) {
        throw std::runtime_error{"Support for " + name + " is not implemented yet."};
      }
    };

  errorNotSupported("Atomic counters", resources.atomic_counters);
  errorNotSupported("Separate images", resources.separate_images);
  errorNotSupported("Separate samplers", resources.separate_samplers);
  errorNotSupported("Storage buffers", resources.storage_buffers);
  errorNotSupported("Subpass inputs", resources.subpass_inputs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::vector<std::shared_ptr<ShaderReflection>> const& stages) {
  for (auto const& stage : stages) {
    merge(*stage);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::vector<ShaderReflection> const& stages) {
  for (auto const& stage : stages) {
    merge(stage);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderReflection::merge(ShaderReflection const& stage) {

  // check that we do not have such a stage already
  if ((VkShaderStageFlags)(mStages & stage.mStages) > 0) {
    throw std::runtime_error{stagesToInfoString(stage.mStages) +
                             " shader stage is already present!"};
  }

  // concatenate stages
  mStages |= stage.mStages;

  // merge mActiveDescriptorSets
  mActiveDescriptorSets.insert(
    stage.mActiveDescriptorSets.begin(), stage.mActiveDescriptorSets.end());

  // combine buffers
  auto mergeBuffers = [](std::vector<Buffer> const& srcBuffers, std::vector<Buffer>& dstBuffers) {

    for (auto const& srcBuffer : srcBuffers) {
      bool merged = false;

      for (auto& dstBuffer : dstBuffers) {

        // there is already a buffer at this binding point
        if (srcBuffer.mBinding == dstBuffer.mBinding && srcBuffer.mSet == dstBuffer.mSet) {

          // check if they have the same type
          if (srcBuffer.mType != dstBuffer.mType) {
            throw std::runtime_error{"Types of Buffers at binding point " +
                                     std::to_string(dstBuffer.mBinding) + " do not match!"};
          }

          // check if they have the same size
          if (srcBuffer.mSize != dstBuffer.mSize) {
            throw std::runtime_error{"Sizes of Buffers at binding point " +
                                     std::to_string(dstBuffer.mBinding) + " do not match!"};
          }

          // check if they have the same packing standards
          if (srcBuffer.mPackingStandard != dstBuffer.mPackingStandard) {
            throw std::runtime_error{"Packing standards of Buffers at binding point " +
                                     std::to_string(dstBuffer.mBinding) + " do not match!"};
          }

          // check if they have the same ranges
          if (srcBuffer.mRanges.size() != dstBuffer.mRanges.size()) {
            throw std::runtime_error{"Ranges of Buffers at binding point " +
                                     std::to_string(dstBuffer.mBinding) + " do not match!"};
          }

          // check if they have the same ranges
          for (size_t i{0}; i < srcBuffer.mRanges.size(); ++i) {
            if (srcBuffer.mRanges[i] != dstBuffer.mRanges[i]) {
              throw std::runtime_error{"Ranges of Buffers at binding point " +
                                       std::to_string(dstBuffer.mBinding) + " do not match!"};
            }

            dstBuffer.mRanges[i].mActiveStages |= srcBuffer.mRanges[i].mActiveStages;
          }

          dstBuffer.mActiveStages |= srcBuffer.mActiveStages;

          merged = true;
          break;
        }
      }

      // this buffer is not part of the combined module yet
      if (!merged) dstBuffers.push_back(srcBuffer);
    }
  };

  // combine samplers
  auto mergeSamplers = [](std::vector<Sampler> const& src, std::vector<Sampler>& dst) {
    for (auto const& srcSampler : src) {
      bool merged = false;
      for (auto& dstSampler : dst) {
        if (srcSampler.mBinding == dstSampler.mBinding) {
          dstSampler.mActiveStages |= srcSampler.mActiveStages;
          merged = true;
          break;
        }
      }

      // this buffer is not part of the combined module yet
      if (!merged) { dst.push_back(srcSampler); }
    }
  };

  mergeBuffers(stage.mPushConstantBuffers, mPushConstantBuffers);
  mergeBuffers(stage.mUniformBuffers, mUniformBuffers);
  mergeSamplers(stage.mSamplers, mSamplers);
  mergeSamplers(stage.mStorageImages, mStorageImages);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::toInfoString() const {
  std::stringstream sstr;

  if (!mUniformBuffers.empty()) {
    sstr << "Uniform Buffers:" << std::endl;
    for (auto const& resource : mUniformBuffers) {
      sstr << resource.toInfoString() << std::endl;
    }
  }

  if (!mPushConstantBuffers.empty()) {
    sstr << "PushConstant Buffers:" << std::endl;
    for (auto const& resource : mPushConstantBuffers) {
      sstr << resource.toInfoString() << std::endl;
    }
  }

  if (!mSamplers.empty()) {
    sstr << "Samplers:" << std::endl;
    for (auto const& resource : mSamplers) {
      sstr << resource.toInfoString() << std::endl;
    }
  }

  if (!mStorageImages.empty()) {
    sstr << "Storage Images:" << std::endl;
    for (auto const& resource : mStorageImages) {
      sstr << resource.toInfoString() << std::endl;
    }
  }

  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::toCppString() const {
  std::stringstream sstr;

  if (!mUniformBuffers.empty()) {
    for (auto const& resource : mUniformBuffers) {
      sstr << resource.toCppString() << std::endl;
    }
  }

  if (!mPushConstantBuffers.empty()) {
    for (auto const& resource : mPushConstantBuffers) {
      sstr << resource.toCppString() << std::endl;
    }
  }

  if (!mSamplers.empty()) {
    for (auto const& resource : mSamplers) {
      sstr << resource.toCppString() << std::endl;
    }
  }

  if (!mStorageImages.empty()) {
    for (auto const& resource : mStorageImages) {
      sstr << resource.toCppString() << std::endl;
    }
  }

  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t ShaderReflection::BufferRange::getInternalPadding() const {
  if (mBaseType != BaseType::eStruct || mMembers.empty()) return 0;

  uint32_t endOfLastMember{mMembers.back().mOffset + mMembers.back().mSize};
  return (mAlignment - (endOfLastMember % mAlignment)) % mAlignment;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t ShaderReflection::BufferRange::getBaseSize() const {
  if (mColumns > 1 && mRows > 1) return mColumns * mRows * mBaseSize;
  return mElements * mBaseSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getTypePrefix() const {
  if (mElements == 1) return "";

  switch (mBaseType) {
  case BaseType::eDouble:
    return "d";
  case BaseType::eInt:
    return "i";
  case BaseType::eUInt:
    return "u";
  default:
    return "";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getElementsPostfix() const {
  if (mColumns > 1 && mRows > 1) {
    if (mColumns == mRows) return std::to_string(mColumns);
    return std::to_string(mColumns) + "x" + std::to_string(mRows);
  }

  if (mElements > 1) return std::to_string(mElements);

  return "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getArrayPostfix() const {
  std::string result;

  for (int i = mArrayLengths.size() - 1; i >= 0; --i) {
    if (mArrayLengths[i] > 0) result += "[" + std::to_string(mArrayLengths[i]) + "]";
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getInfoType() const {
  if (mColumns > 1 && mRows > 1) return getTypePrefix() + "mat" + getElementsPostfix();
  if (mElements > 1) return getTypePrefix() + "vec" + getElementsPostfix();

  switch (mBaseType) {
  case BaseType::eInt:
    return "int";
  case BaseType::eUInt:
    return "uint";
  case BaseType::eFloat:
    return "float";
  case BaseType::eDouble:
    return "double";
  case BaseType::eStruct:
    return mTypeName;
  default:
    return "unknown";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getCppType() const {

  // It can be necessary that the cpp is a bit larger than the spirv type when padding is required.
  // Therefore we create a copy and modify it in such a way that a padding rules are fullfilled.
  // Only modify base types. Structs need to be padded inside.
  if (mBaseType != BaseType::eUnknown && mBaseType != BaseType::eStruct) {

    // First modification can be neccessary when the matrix stride is larger than the row count. In
    // this case we should use the matrix stride value instead
    if (mColumns > 1 && mRows > 1 && mRows < mMatrixStride / mBaseSize) {
      BufferRange copy{*this};
      copy.mRows = copy.mMatrixStride / copy.mBaseSize;
      return copy.getCppType();
    }

    // Next modification should occur when base type array elements are smaller than the array
    // stride. In this case we should use a larger glm type to fill the padding.
    if (getBaseSize() < mArrayStride) {
      BufferRange copy{*this};

      // Matrix types should increase the column count accordingly
      if (mColumns > 1 && mRows > 1) {
        copy.mColumns = copy.mArrayStride / copy.mBaseSize / copy.mRows;
        return copy.getCppType();
      }

      // Scalar or vector types should increase the amount of elements
      copy.mElements = copy.mArrayStride / copy.mBaseSize;
      return copy.getCppType();
    }
  }

  // All required padding / stride issues should be resolved now. For matrix types return glm
  // matrices, for vector types glm vectors
  if (mColumns > 1 && mRows > 1) {
    return "glm::" + getTypePrefix() + "mat" + getElementsPostfix();
  }
  if (mElements > 1) { return "glm::" + getTypePrefix() + "vec" + getElementsPostfix(); }

  // For base types, return the C++ equivavlent.
  switch (mBaseType) {
  case BaseType::eInt:
    return "int";
  case BaseType::eUInt:
    return "unsigned";
  case BaseType::eFloat:
    return "float";
  case BaseType::eDouble:
    return "double";
  case BaseType::eStruct:
    return mTypeName;
  default:
    return "unknown";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Buffer::toInfoString() const {
  std::stringstream sstr;
  sstr << " - " << mType << " " << mName << " (Stages: " << stagesToInfoString(mActiveStages) << ")"
       << std::endl;
  sstr << "   Size:    " << mSize << std::endl;
  sstr << "   Binding: " << mBinding << std::endl;
  sstr << "   Set:     " << mSet << std::endl;

  for (auto const& range : mRanges) {
    sstr << "   - " << range.getInfoType() << " " << range.mName << range.getArrayPostfix()
         << " (Stages: " << stagesToInfoString(range.mActiveStages) << ")" << std::endl;
    sstr << "     Size:         " << range.mSize << std::endl;
    sstr << "     Offset:       " << range.mOffset << std::endl;
    sstr << "     Alignment:    " << range.mAlignment << std::endl;

    if (range.mArrayStride > 0) sstr << "     ArrayStride:  " << range.mArrayStride << std::endl;
    if (range.mMatrixStride > 0) sstr << "     MatrixStride: " << range.mMatrixStride << std::endl;

    if (range.mBaseType == BufferRange::BaseType::eStruct) {
      sstr << "     - Members: " << std::endl;
      for (auto const& member : range.mMembers) {
        sstr << "       - " << member.getInfoType() << " " << member.mName
             << member.getArrayPostfix() << std::endl;
        sstr << "         Size:         " << member.mSize << std::endl;
        sstr << "         Offset:       " << member.mOffset << std::endl;
        sstr << "         Alignment:    " << member.mAlignment << std::endl;

        if (member.mArrayStride > 0)
          sstr << "         ArrayStride:  " << member.mArrayStride << std::endl;
        if (member.mMatrixStride > 0)
          sstr << "         MatrixStride: " << member.mMatrixStride << std::endl;
      }
    }
  }

  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Buffer::toCppString() const {
  std::stringstream sstr;
  sstr << "struct " << mType << " {" << std::endl;
  sstr << std::endl;
  sstr << "  // reflection information" << std::endl;
  sstr << "  static vk::ShaderStageFlags getActiveStages()  { return "
       << stagesToCppString(mActiveStages) << "; }" << std::endl;
  sstr << "  static uint32_t             getBindingPoint()  { return " << mBinding << "; }"
       << std::endl;
  sstr << "  static uint32_t             getDescriptorSet() { return " << mSet << "; }"
       << std::endl;
  sstr << std::endl;

  // collect all structs
  std::map<std::string, BufferRange>                   structs;
  std::function<void(std::vector<BufferRange> const&)> collectStructs =
    [&structs, &collectStructs](std::vector<BufferRange> const& ranges) {
      for (auto const& range : ranges) {
        if (range.mBaseType == BufferRange::BaseType::eStruct) {
          structs[range.mTypeName] = range;
          collectStructs(range.mMembers);
        }
      }
    };

  collectStructs(mRanges);

  if (structs.size() > 0) {

    sstr << "  // structs used in this block" << std::endl;

    // firts emit forward declaration for all structs
    for (auto const& s : structs) {
      sstr << "  struct " << s.first << ";" << std::endl;
    }

    sstr << std::endl;

    // then emit definitions of all structs
    for (auto const& s : structs) {
      sstr << "  struct " << s.first << " {" << std::endl;
      uint32_t paddingCounter{0};

      // loop over all member of the struct
      auto const& structMembers{s.second.mMembers};

      for (size_t i{0}; i < structMembers.size(); ++i) {

        // emit the member
        sstr << "    " << structMembers[i].getCppType() << " " << structMembers[i].mName
             << structMembers[i].getArrayPostfix() << ";" << std::endl;

        // add padding after each member if required
        uint32_t requiredPadding{0};
        uint32_t nextOffset{0};
        uint32_t selfOffset{structMembers[i].mOffset};
        uint32_t selfSize{structMembers[i].mSize};

        if (
          structMembers[i].mBaseType == BufferRange::BaseType::eStruct &&
          structMembers[i].mArrayStride == 0) {
          // structs have padding to their base alignment "built in" - herefore we need to add this
          // to the size of the range (only if it's not an array of structs, in this case the
          // padding is already included in the mSize member)
          selfSize += structMembers[i].getInternalPadding();
        }

        if (i < structMembers.size() - 1) {
          // the offset of the next member is easy to get if there is a next meber
          nextOffset = structMembers[i + 1].mOffset;
        } else {
          // at the end of a struct we add padding until the base alignment is met
          nextOffset = selfOffset + selfSize + s.second.getInternalPadding();
        }

        requiredPadding = (nextOffset - selfOffset - selfSize) / sizeof(float);

        while (requiredPadding > 0) {
          sstr << "    float _padding" << ++paddingCounter << ";" << std::endl;
          --requiredPadding;
        }
      }
      sstr << "  };" << std::endl;
      sstr << std::endl;
    }
  }

  sstr << "  // struct members" << std::endl;

  uint32_t paddingCounter{0};

  for (size_t i{0}; i < mRanges.size(); ++i) {
    sstr << "  " << mRanges[i].getCppType() << " " << mRanges[i].mName
         << mRanges[i].getArrayPostfix() << ";" << std::endl;

    // add padding between all ranges but not after the last
    if (i < mRanges.size() - 1) {
      uint32_t requiredPadding{0};
      uint32_t nextOffset{mRanges[i + 1].mOffset};
      uint32_t selfOffset{mRanges[i].mOffset};
      uint32_t selfSize{mRanges[i].mSize};

      if (mRanges[i].mBaseType == BufferRange::BaseType::eStruct && mRanges[i].mArrayStride == 0) {
        // structs have padding to their base alignment "built in", therefore we need to add this
        // to the size of the range (only if it's not an array of structs, in this case the padding
        // is already included in the mSize member)
        selfSize += mRanges[i].getInternalPadding();
      }

      requiredPadding = (nextOffset - selfOffset - selfSize) / sizeof(float);

      // print a float for each padding required
      while (requiredPadding > 0) {
        sstr << "  float _padding" << ++paddingCounter << ";" << std::endl;
        --requiredPadding;
      }
    }
  }

  sstr << "};" << std::endl;
  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Sampler::toInfoString() const {
  std::stringstream sstr;
  sstr << " - Name: " << mName << " (Stages: " << stagesToInfoString(mActiveStages) << ")"
       << std::endl;
  sstr << "   Binding: " << mBinding << std::endl;
  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Sampler::toCppString() const {
  std::stringstream sstr;
  sstr << "// combined image sampler" << std::endl;
  sstr << "struct " << mName << " {" << std::endl;
  sstr << "  static vk::ShaderStageFlags getActiveStages()  { return "
       << stagesToCppString(mActiveStages) << "; }" << std::endl;
  sstr << "  static uint32_t             getBindingPoint()  { return " << mBinding << "; }"
       << std::endl;
  sstr << "  static uint32_t             getDescriptorSet() { return " << mSet << "; }"
       << std::endl;
  sstr << "};" << std::endl;
  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Illusion::Graphics
