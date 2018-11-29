////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GltfModel.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "Texture.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include <unordered_set>

// std::vector<PBRMaterial> materials;

// for (auto material : model.materials) {
//   auto getTextureIndex = [](tinygltf::ParameterMap const& matParams, std::string const& name)
//   {
//     auto texIt = matParams.find(name);
//     if (texIt == matParams.end()) return -1;
//     auto idxIt = texIt->second.json_double_value.find("index");
//     if (idxIt == texIt->second.json_double_value.end()) return -1;
//     return static_cast<int>(idxIt->second);
//   };

//   PBRMaterial m;
//   {
//     int index{getTextureIndex(material.values, "baseColorTexture")};
//     if (index >= 0) {
//       m.mBaseColorTexture = Illusion::Graphics::TinyGLTF::createTexture(engine, model,
//       index);
//     }
//   }
//   {
//     int index{getTextureIndex(material.values, "metallicRoughnessTexture")};
//     if (index >= 0) {
//       m.mMetallicRoughnessTexture =
//         Illusion::Graphics::TinyGLTF::createTexture(engine, model, index);
//     }
//   }
//   {
//     int index{getTextureIndex(material.additionalValues, "normalTexture")};
//     if (index >= 0) {
//       m.mNormalTexture = Illusion::Graphics::TinyGLTF::createTexture(engine, model, index);
//     }
//   }
//   {
//     int index{getTextureIndex(material.additionalValues, "occlusionTexture")};
//     if (index >= 0) {
//       m.mOcclusionTexture = Illusion::Graphics::TinyGLTF::createTexture(engine, model,
//       index);
//     }
//   }
//   {
//     int index{getTextureIndex(material.additionalValues, "emissiveTexture")};
//     if (index >= 0) {
//       m.mEmissiveTexture = Illusion::Graphics::TinyGLTF::createTexture(engine, model, index);
//     }
//   }

//   materials.push_back(m);
// }

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

GltfModel::GltfModel(
  std::shared_ptr<Illusion::Graphics::Device> const& device, std::string const& file)
  : mDevice(device)
  , mFile(file) {

  std::string        extension{file.substr(file.find_last_of('.'))};
  std::string        error, warn;
  bool               success = false;
  tinygltf::TinyGLTF loader;

  if (extension == ".glb") {
    ILLUSION_TRACE << "Loading binary file " << file << "..." << std::endl;
    success = loader.LoadBinaryFromFile(&mGLTF, &error, &warn, file);
  } else if (extension == ".gltf") {
    ILLUSION_TRACE << "Loading ascii file " << file << "..." << std::endl;
    success = loader.LoadASCIIFromFile(&mGLTF, &error, &warn, file);
  } else {
    throw std::runtime_error{"Unknown extension " + extension};
  }

  if (!error.empty()) { throw std::runtime_error{"Error loading file " + file + ": " + error}; }
  if (!success) { throw std::runtime_error{"Error loading file " + file}; }

  loadData();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::predraw(vk::CommandBuffer const& cmd) {
  // for (auto& buffer : mUniformBuffers) {
  //   buffer.update(cmd);
  // }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::draw(
  vk::CommandBuffer const&           cmd,
  std::shared_ptr<RenderPass> const& renderPass,
  uint32_t                           subPass,
  glm::dmat4                         modelMatrix) {

  // std::function<void(int, glm::dmat4)> drawNode =
  //   [cmd, subPass, this, &renderPass, &drawNode](int n, glm::dmat4 modelMatrix) {
  //     auto const& node = mGLTF.nodes[n];

  //     if (node.matrix.size() > 0) {
  //       modelMatrix *= glm::make_mat4(node.matrix.data());
  //     } else {
  //       if (node.translation.size() > 0) {
  //         modelMatrix = glm::translate(modelMatrix, glm::make_vec3(node.translation.data()));
  //       }
  //       if (node.rotation.size() > 0) {
  //         glm::dquat quaternion(glm::make_quat(node.rotation.data()));
  //         modelMatrix = glm::rotate(modelMatrix, glm::angle(quaternion), glm::axis(quaternion));
  //       }
  //       if (node.scale.size() > 0) {
  //         modelMatrix = glm::scale(modelMatrix, glm::make_vec3(node.scale.data()));
  //       }
  //     }

  //     // draw
  //     if (node.mesh >= 0) {
  //       auto const& mesh = mGLTF.meshes[node.mesh];

  //       for (auto const& primitive : mesh.primitives) {

  //         // bind pipeline
  //         -------------------------------------------------------------------------

  //         std::unordered_set<uint32_t> uniqueBufferViews;
  //         for (auto const& attribute : primitive.attributes) {
  //           auto const& accessor = mGLTF.accessors[attribute.second];
  //           uniqueBufferViews.insert(accessor.bufferView);
  //         }

  //         std::vector<vk::VertexInputBindingDescription> inputBindings;
  //         for (auto const& index : uniqueBufferViews) {
  //           auto const& bufferView = mGLTF.bufferViews[index];
  //           uint32_t    stride     = bufferView.byteStride;
  //           inputBindings.push_back({index, stride, vk::VertexInputRate::eVertex});
  //         }

  //         std::vector<vk::VertexInputAttributeDescription> inputAttributes;
  //         for (auto const& attribute : primitive.attributes) {
  //           int location = -1;
  //           if (attribute.first == "POSITION")
  //             location = 0;
  //           else if (attribute.first == "NORMAL")
  //             location = 1;
  //           else if (attribute.first == "TANGENT")
  //             location = 2;
  //           else if (attribute.first == "TEXCOORD_0")
  //             location = 3;

  //           if (location < 0) {
  //             throw std::runtime_error{"Unsupported attribute " + attribute.first};
  //           }

  //           auto const& accessor = mGLTF.accessors[attribute.second];

  //           uint32_t   binding = accessor.bufferView;
  //           vk::Format format  = convertFormat(accessor.type, accessor.componentType);

  //           inputAttributes.push_back(
  //             {(uint32_t)location, binding, format, (uint32_t)accessor.byteOffset});
  //         }

  //         vk::PrimitiveTopology topology = convertPrimitiveTopology(primitive.mode);
  //         mMaterial->bind(cmd, renderPass, subPass, topology, inputBindings, inputAttributes);

  //         // update uniforms
  //         -----------------------------------------------------------------------
  //         mMaterial->getLayout()->useDescriptorSet(cmd, mDescriptorSets[primitive.material], 1);

  //         // update push constants
  //         -----------------------------------------------------------------
  //         mMaterial->getLayout()->setPushConstant(
  //           cmd, vk::ShaderStageFlagBits::eVertex, glm::mat4(modelMatrix));

  //         // bind vertex buffers
  //         ------------------------------------------------------------------- for (auto const&
  //         index : uniqueBufferViews) {
  //           auto const& bufferView = mGLTF.bufferViews[index];
  //           auto const& buffer     = mBuffers[bufferView.buffer];

  //           cmd.bindVertexBuffers(index, *buffer->mBuffer, bufferView.byteOffset);
  //         }

  //         // bind index buffer
  //         --------------------------------------------------------------------- auto const&
  //         indexAccessor   = mGLTF.accessors[primitive.indices]; auto const&   indexBufferView =
  //         mGLTF.bufferViews[indexAccessor.bufferView]; auto const&   indexBuffer     =
  //         mBuffers[indexBufferView.buffer]; vk::IndexType indexType       =
  //         vk::IndexType::eUint16;

  //         if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
  //           indexType = vk::IndexType::eUint32;
  //         }

  //         cmd.bindIndexBuffer(*indexBuffer->mBuffer, indexBufferView.byteOffset, indexType);

  //         // issue draw command
  //         --------------------------------------------------------------------
  //         cmd.drawIndexed(indexAccessor.count, 1, 0, 0, 0);
  //       }
  //     }

  //     // draw children
  //     for (int n : node.children) {
  //       drawNode(n, modelMatrix);
  //     }

  //   };

  // for (int n : mGLTF.scenes[mGLTF.defaultScene].nodes) {
  //   drawNode(n, modelMatrix);
  // }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::printInfo() const {
  // clang-format off
  ILLUSION_MESSAGE << "Information for " << mFile << ":" << std::endl;
  ILLUSION_MESSAGE << "accessors ........... " << mGLTF.accessors.size() << std::endl;
  for (auto const& o : mGLTF.accessors) {
    ILLUSION_MESSAGE << " |.. " << o.name 
                     << " bufferView: " << o.bufferView 
                     << " byteOffset: " << o.byteOffset 
                     << " normalized: " << o.normalized 
                     << " componentType: " << o.componentType 
                     << " count: " << o.count 
                     << " type: " << o.type 
                     << std::endl;
  }

  ILLUSION_MESSAGE << "animations .......... " << mGLTF.animations.size() << std::endl;
  ILLUSION_MESSAGE << "buffers ............. " << mGLTF.buffers.size() << std::endl;
  ILLUSION_MESSAGE << "bufferViews ......... " << mGLTF.bufferViews.size() << std::endl;
  for (auto const& o : mGLTF.bufferViews) {
    ILLUSION_MESSAGE << " |.. " << o.name 
                     << " buffer: " << o.buffer 
                     << " byteOffset: " << o.byteOffset 
                     << " byteLength: " << o.byteLength 
                     << " byteStride: " << o.byteStride 
                     << " target: " << o.target 
                     << std::endl;
  }

  ILLUSION_MESSAGE << "materials ........... " << mGLTF.materials.size() << std::endl;
  ILLUSION_MESSAGE << "meshes .............. " << mGLTF.meshes.size() << std::endl;
  for (auto const& o : mGLTF.meshes) {
    ILLUSION_MESSAGE << " |.. " << o.name << " primitives: " << o.primitives.size()<< std::endl;
    for (auto const& p : o.primitives) {
      ILLUSION_MESSAGE << " |  |.. index accessor: " << p.indices 
                       << " material: " << p.material 
                       << " mode: " << p.mode 
                       << " attributes: " << p.attributes.size() 
                       << std::endl;
      for (auto const& a : p.attributes) {
        ILLUSION_MESSAGE << " |  |  |.. " << a.first << ": accessor " << a.second << std::endl;
      }
    }
  }

  ILLUSION_MESSAGE << "nodes ............... " << mGLTF.nodes.size() << std::endl;
  ILLUSION_MESSAGE << "textures ............ " << mGLTF.textures.size() << std::endl;
  for (auto const& o : mGLTF.textures) {
    ILLUSION_MESSAGE << " |.. sampler: " << o.sampler << " image: " << o.source << std::endl;
  }

  ILLUSION_MESSAGE << "images .............. " << mGLTF.images.size() << std::endl;
  for (auto const& o : mGLTF.images) {
    ILLUSION_MESSAGE << " |.. " << o.uri << " " << o.width << "x" << o.height << std::endl;
  }

  ILLUSION_MESSAGE << "skins ............... " << mGLTF.skins.size() << std::endl;
  ILLUSION_MESSAGE << "samplers ............ " << mGLTF.samplers.size() << std::endl;
  ILLUSION_MESSAGE << "cameras ............. " << mGLTF.cameras.size() << std::endl;
  ILLUSION_MESSAGE << "scenes .............. " << mGLTF.scenes.size() << std::endl;
  for (auto const& s : mGLTF.scenes) {
    ILLUSION_MESSAGE << " |.. " << s.name << std::endl;
    for (auto const& r : s.nodes) {
      auto const n{mGLTF.nodes[r]};
      ILLUSION_MESSAGE << "   |.. " << n.name << std::endl;
    }
  }

  ILLUSION_MESSAGE << "defaultScene ........ " << mGLTF.defaultScene << std::endl;
  ILLUSION_MESSAGE << "lights .............. " << mGLTF.lights.size() << std::endl;
  ILLUSION_MESSAGE << "extensionsUsed ...... " << mGLTF.extensionsUsed.size() << std::endl;
  for (auto const& o : mGLTF.extensionsUsed) {
    ILLUSION_MESSAGE << " |.. " << o << std::endl;
  }

  ILLUSION_MESSAGE << "extensionsRequired .. " << mGLTF.extensionsRequired.size()  << std::endl;
  for (auto const& o : mGLTF.extensionsRequired) {
    ILLUSION_MESSAGE << " |..  " << o << std::endl;
  }
  // clang-format on
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::loadData() {
  // calculate missing byteStrides
  for (size_t i{0}; i < mGLTF.accessors.size(); ++i) {
    auto const& accessor   = mGLTF.accessors[i];
    auto&       bufferView = mGLTF.bufferViews[accessor.bufferView];
    bufferView.byteStride  = accessor.ByteStride(bufferView);
  }

  for (size_t i{0}; i < mGLTF.textures.size(); ++i) {
    mTextures.push_back(createTexture(i));
  }

  for (size_t i{0}; i < mGLTF.buffers.size(); ++i) {
    mBuffers.push_back(createBuffer(i));
  }

  for (size_t i{0}; i < mGLTF.materials.size(); ++i) {
    // mDescriptorSets.push_back(mMaterial->getLayout()->allocateDescriptorSet(1));
    // mUniformBuffers.push_back(UniformBuffer<PBRMaterialUniforms>(mEngine));

    // mUniformBuffers[i].color = glm::vec3(0, 1, 1);
    // mUniformBuffers[i].bind(mDescriptorSets[i]);

    // int index = getTextureIndex(i, "baseColorTexture");
    // if (index < 0) { throw std::runtime_error{"No 'baseColorTexture' defined!"}; }

    // vk::DescriptorImageInfo imageInfo;
    // imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    // imageInfo.imageView   = *mTextures[index]->getImageView();
    // imageInfo.sampler     = *mTextures[index]->getSampler();

    // vk::WriteDescriptorSet info;
    // info.dstSet          = mDescriptorSets[i];
    // info.dstBinding      = 1;
    // info.dstArrayElement = 0;
    // info.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
    // info.descriptorCount = 1;
    // info.pImageInfo      = &imageInfo;

    // mEngine->getDevice()->updateDescriptorSets(info, nullptr);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Filter GltfModel::convertFilter(int value) {
  switch (value) {
  case TINYGLTF_TEXTURE_FILTER_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
    return vk::Filter::eNearest;
  case TINYGLTF_TEXTURE_FILTER_LINEAR:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
    return vk::Filter::eNearest;
  }

  throw std::runtime_error{"Invalid filter mode " + std::to_string(value)};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerMipmapMode GltfModel::convertSamplerMipmapMode(int value) {
  switch (value) {
  case TINYGLTF_TEXTURE_FILTER_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
    return vk::SamplerMipmapMode::eNearest;
  case TINYGLTF_TEXTURE_FILTER_LINEAR:
  case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
    return vk::SamplerMipmapMode::eLinear;
  }

  throw std::runtime_error{"Invalid sampler mipmap mode " + std::to_string(value)};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SamplerAddressMode GltfModel::convertSamplerAddressMode(int value) {
  switch (value) {
  case TINYGLTF_TEXTURE_WRAP_REPEAT:
    return vk::SamplerAddressMode::eRepeat;
  case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
    return vk::SamplerAddressMode::eClampToEdge;
  case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
    return vk::SamplerAddressMode::eMirroredRepeat;
  }

  throw std::runtime_error{"Invalid sampler address mode " + std::to_string(value)};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Format GltfModel::convertFormat(int type, int componentType) {

  switch (componentType) {
  case TINYGLTF_COMPONENT_TYPE_BYTE: {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return vk::Format::eR8Sint;
    case TINYGLTF_TYPE_VEC2:
      return vk::Format::eR8G8Sint;
    case TINYGLTF_TYPE_VEC3:
      return vk::Format::eR8G8B8Sint;
    case TINYGLTF_TYPE_VEC4:
      return vk::Format::eR8G8B8A8Sint;
    }
  }

  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return vk::Format::eR8Uint;
    case TINYGLTF_TYPE_VEC2:
      return vk::Format::eR8G8Uint;
    case TINYGLTF_TYPE_VEC3:
      return vk::Format::eR8G8B8Uint;
    case TINYGLTF_TYPE_VEC4:
      return vk::Format::eR8G8B8A8Uint;
    }
  }

  case TINYGLTF_COMPONENT_TYPE_SHORT: {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return vk::Format::eR16Sint;
    case TINYGLTF_TYPE_VEC2:
      return vk::Format::eR16G16Sint;
    case TINYGLTF_TYPE_VEC3:
      return vk::Format::eR16G16B16Sint;
    case TINYGLTF_TYPE_VEC4:
      return vk::Format::eR16G16B16A16Sint;
    }
  }

  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return vk::Format::eR16Uint;
    case TINYGLTF_TYPE_VEC2:
      return vk::Format::eR16G16Uint;
    case TINYGLTF_TYPE_VEC3:
      return vk::Format::eR16G16B16Uint;
    case TINYGLTF_TYPE_VEC4:
      return vk::Format::eR16G16B16A16Uint;
    }
  }

  case TINYGLTF_COMPONENT_TYPE_INT: {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return vk::Format::eR32Sint;
    case TINYGLTF_TYPE_VEC2:
      return vk::Format::eR32G32Sint;
    case TINYGLTF_TYPE_VEC3:
      return vk::Format::eR32G32B32Sint;
    case TINYGLTF_TYPE_VEC4:
      return vk::Format::eR32G32B32A32Sint;
    }
  }

  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return vk::Format::eR32Uint;
    case TINYGLTF_TYPE_VEC2:
      return vk::Format::eR32G32Uint;
    case TINYGLTF_TYPE_VEC3:
      return vk::Format::eR32G32B32Uint;
    case TINYGLTF_TYPE_VEC4:
      return vk::Format::eR32G32B32A32Uint;
    }
  }

  case TINYGLTF_COMPONENT_TYPE_FLOAT: {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return vk::Format::eR32Sfloat;
    case TINYGLTF_TYPE_VEC2:
      return vk::Format::eR32G32Sfloat;
    case TINYGLTF_TYPE_VEC3:
      return vk::Format::eR32G32B32Sfloat;
    case TINYGLTF_TYPE_VEC4:
      return vk::Format::eR32G32B32A32Sfloat;
    }
  }
  }

  throw std::runtime_error{"Invalid format combination " + std::to_string(type) + " / " +
                           std::to_string(componentType)};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PrimitiveTopology GltfModel::convertPrimitiveTopology(int value) {
  switch (value) {
  case TINYGLTF_MODE_POINTS:
    return vk::PrimitiveTopology::ePointList;
  case TINYGLTF_MODE_LINE:
    return vk::PrimitiveTopology::eLineStrip;
  case TINYGLTF_MODE_TRIANGLES:
    return vk::PrimitiveTopology::eTriangleList;
  case TINYGLTF_MODE_TRIANGLE_STRIP:
    return vk::PrimitiveTopology::eTriangleStrip;
  case TINYGLTF_MODE_TRIANGLE_FAN:
    return vk::PrimitiveTopology::eTriangleFan;
  }

  throw std::runtime_error{"Invalid primitive topology " + std::to_string(value)};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Illusion::Graphics::BackedBuffer> GltfModel::createBuffer(int index) {
  vk::BufferUsageFlags usage;

  for (auto const& view : mGLTF.bufferViews) {
    if (view.buffer == index) {
      if (view.target == TINYGLTF_TARGET_ARRAY_BUFFER)
        usage |= vk::BufferUsageFlagBits::eVertexBuffer;
      else if (view.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER)
        usage |= vk::BufferUsageFlagBits::eIndexBuffer;
    }
  }

  if (!usage) {
    ILLUSION_WARNING << "No target information given for buffer " << index << " in file " << mFile
                     << ". Assuming vertex and index buffer." << std::endl;
    usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer;
  }

  return mDevice->createBackedBuffer(
    mGLTF.buffers[index].data.size(),
    usage,
    vk::MemoryPropertyFlagBits::eDeviceLocal,
    mGLTF.buffers[index].data.data());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Illusion::Graphics::Texture> GltfModel::createTexture(int index) const {

  tinygltf::Sampler sampler;

  if (mGLTF.textures[index].sampler >= 0) {
    sampler = mGLTF.samplers[mGLTF.textures[index].sampler];
  } else {
    sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
    sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
    sampler.wrapS     = TINYGLTF_TEXTURE_WRAP_REPEAT;
    sampler.wrapT     = TINYGLTF_TEXTURE_WRAP_REPEAT;
  }

  tinygltf::Image image;

  if (mGLTF.textures[index].source >= 0) {
    image = mGLTF.images[mGLTF.textures[index].source];
  } else {
    throw std::runtime_error{"No image source given"};
  }

  vk::SamplerCreateInfo info;
  info.magFilter               = convertFilter(sampler.magFilter);
  info.minFilter               = convertFilter(sampler.minFilter);
  info.addressModeU            = convertSamplerAddressMode(sampler.wrapS);
  info.addressModeV            = convertSamplerAddressMode(sampler.wrapT);
  info.addressModeW            = vk::SamplerAddressMode::eRepeat;
  info.anisotropyEnable        = true;
  info.maxAnisotropy           = 16;
  info.borderColor             = vk::BorderColor::eIntOpaqueBlack;
  info.unnormalizedCoordinates = false;
  info.compareEnable           = false;
  info.compareOp               = vk::CompareOp::eAlways;
  info.mipmapMode              = convertSamplerMipmapMode(sampler.minFilter);
  info.mipLodBias              = 0.0f;
  info.minLod                  = 0.0f;
  info.maxLod                  = 0;

  // if no image data has been loaded, try loading it on our own
  if (image.image.empty()) { return Texture::createFromFile(mDevice, image.uri, info); }

  // if there is image data, create an appropriate texture object for it
  uint32_t channels = image.image.size() / image.width / image.height;

  return Texture::create2D(
    mDevice,
    image.width,
    image.height,
    channels == 3 ? vk::Format::eR8G8B8Unorm : vk::Format::eR8G8B8A8Unorm,
    vk::ImageUsageFlagBits::eSampled,
    info,
    image.image.size(),
    (void*)image.image.data());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int GltfModel::getTextureIndex(int materialIndex, std::string const& name) {
  auto const& material = mGLTF.materials[materialIndex];
  auto        texIt    = material.values.find(name);
  if (texIt == material.values.end()) return -1;
  auto idxIt = texIt->second.json_double_value.find("index");
  if (idxIt == texIt->second.json_double_value.end()) return -1;
  return static_cast<int>(idxIt->second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
