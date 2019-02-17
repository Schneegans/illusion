////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GltfModel.hpp"

#include <Illusion/Graphics/CoherentBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Shader.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////

GltfModel::GltfModel(std::string const& name, Illusion::Graphics::DevicePtr const& device,
    std::string const& fileName, Illusion::Graphics::Gltf::LoadOptions const& options,
    Illusion::Graphics::FrameResourceIndexPtr const& frameIndex)
    : mDevice(device)
    , mModel(Illusion::Graphics::Gltf::Model::create(name, device, fileName, options))
    , mShader(Illusion::Graphics::Shader::createFromFiles("PBRShader", device,
          {"data/GltfViewer/shaders/GltfShader.vert", "data/GltfViewer/shaders/GltfShader.frag"}))
    , mSkinBuffers(frameIndex,
          [=](uint32_t) {
            return std::unordered_map<Illusion::Graphics::Gltf::SkinPtr,
                Illusion::Graphics::CoherentBufferPtr>();
          })
    , mEmptySkinBuffer(Illusion::Graphics::CoherentBuffer::create(
          "EmptySkinUniformBuffer", mDevice, 1, vk::BufferUsageFlagBits::eUniformBuffer)) {

  auto      modelBBox   = mModel->getRoot()->getBoundingBox();
  float     modelSize   = glm::length(modelBBox.mMin - modelBBox.mMax);
  glm::vec3 modelCenter = (modelBBox.mMin + modelBBox.mMax) * 0.5f;
  mModelMatrix          = glm::scale(glm::vec3(1.f / modelSize));
  mModelMatrix          = glm::translate(mModelMatrix, -modelCenter);

  mModel->printInfo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::update(double time, int32_t animation) {
  if (static_cast<size_t>(animation) < mModel->getAnimations().size()) {
    auto const& anim               = mModel->getAnimations()[animation];
    float       modelAnimationTime = std::fmod(static_cast<float>(time), anim->mEnd - anim->mStart);
    modelAnimationTime += anim->mStart;
    mModel->setAnimationTime(animation, modelAnimationTime);
  }

  for (auto const& skin : mModel->getSkins()) {

    SkinUniforms skinBuffer{};
    auto         jointMatrices = skin->getJointMatrices();

    for (size_t i(0); i < jointMatrices.size() && i < 256; ++i) {
      skinBuffer.mJointMatrices[i] = jointMatrices[i];
    }

    if (mSkinBuffers.current().find(skin) == mSkinBuffers.current().end()) {
      mSkinBuffers.current()[skin] = Illusion::Graphics::CoherentBuffer::create("SkinUniformBuffer",
          mDevice, sizeof(SkinUniforms), vk::BufferUsageFlagBits::eUniformBuffer);
    }

    mSkinBuffers.current().at(skin)->updateData(skinBuffer);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::draw(Illusion::Graphics::CommandBufferPtr const& cmd, glm::mat4 const& viewMatrix) {
  cmd->setShader(mShader);
  cmd->graphicsState().setVertexInputAttributes(
      Illusion::Graphics::Gltf::Model::getVertexInputAttributes());
  cmd->graphicsState().setVertexInputBindings(
      Illusion::Graphics::Gltf::Model::getVertexInputBindings());

  cmd->bindVertexBuffers(0, {mModel->getVertexBuffer()});
  cmd->bindIndexBuffer(mModel->getIndexBuffer(), 0, vk::IndexType::eUint32);

  drawNodes(cmd, mModel->getRoot()->mChildren, viewMatrix, false);
  drawNodes(cmd, mModel->getRoot()->mChildren, viewMatrix, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::drawNodes(Illusion::Graphics::CommandBufferPtr const&   cmd,
    std::vector<std::shared_ptr<Illusion::Graphics::Gltf::Node>> const& nodes,
    glm::mat4 const& viewMatrix, bool doAlphaBlending) {

  cmd->graphicsState().setBlendAttachments({{doAlphaBlending}});

  for (auto const& n : nodes) {

    glm::mat4 nodeMatrix = n->mGlobalTransform;

    if (n->mSkin) {
      auto buffer = mSkinBuffers.current()[n->mSkin]->getBuffer();
      cmd->bindingState().setUniformBuffer(buffer, sizeof(SkinUniforms), 0, 2, 0);
    } else {
      cmd->bindingState().setUniformBuffer(mEmptySkinBuffer->getBuffer(), 1, 0, 2, 0);
    }

    if (n->mMesh) {

      for (auto const& p : n->mMesh->mPrimitives) {

        if (p.mMaterial->mDoAlphaBlending == doAlphaBlending) {
          PushConstants pushConstants{};
          pushConstants.mModelMatrix                = mModelMatrix * nodeMatrix;
          pushConstants.mAlbedoFactor               = p.mMaterial->mAlbedoFactor;
          pushConstants.mEmissiveFactor             = p.mMaterial->mEmissiveFactor;
          pushConstants.mSpecularGlossinessWorkflow = p.mMaterial->mSpecularGlossinessWorkflow;
          pushConstants.mMetallicRoughnessFactor    = p.mMaterial->mMetallicRoughnessFactor;
          pushConstants.mNormalScale                = p.mMaterial->mNormalScale;
          pushConstants.mOcclusionStrength          = p.mMaterial->mOcclusionStrength;
          pushConstants.mAlphaCutoff                = p.mMaterial->mAlphaCutoff;
          pushConstants.mVertexAttributes           = static_cast<int32_t>(p.mVertexAttributes);
          cmd->pushConstants(pushConstants);

          cmd->bindingState().setTexture(p.mMaterial->mAlbedoTexture, 3, 0);
          cmd->bindingState().setTexture(p.mMaterial->mMetallicRoughnessTexture, 3, 1);
          cmd->bindingState().setTexture(p.mMaterial->mNormalTexture, 3, 2);
          cmd->bindingState().setTexture(p.mMaterial->mOcclusionTexture, 3, 3);
          cmd->bindingState().setTexture(p.mMaterial->mEmissiveTexture, 3, 4);
          cmd->graphicsState().setTopology(p.mTopology);
          cmd->graphicsState().setCullMode(p.mMaterial->mDoubleSided ? vk::CullModeFlagBits::eNone
                                                                     : vk::CullModeFlagBits::eBack);
          cmd->drawIndexed(p.mIndexCount, 1, p.mIndexOffset, 0, 0);
        }
      }
    }

    drawNodes(cmd, n->mChildren, viewMatrix, doAlphaBlending);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
