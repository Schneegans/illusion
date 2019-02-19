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

  // Our mModelMatrix scales and translates the model in such a way that it is approximately
  // centered on the screen.
  auto      modelBBox   = mModel->getRoot()->getBoundingBox();
  float     modelSize   = glm::length(modelBBox.mMin - modelBBox.mMax);
  glm::vec3 modelCenter = (modelBBox.mMin + modelBBox.mMax) * 0.5f;
  mModelMatrix          = glm::scale(glm::vec3(1.f / modelSize));
  mModelMatrix          = glm::translate(mModelMatrix, -modelCenter);

  // We print some information on the loaded model.
  mModel->printInfo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GltfModel::update(double time, int32_t animation) {

  // First update the animation state of all nodes.
  if (static_cast<size_t>(animation) < mModel->getAnimations().size()) {
    auto const& anim = mModel->getAnimations()[animation];

    // Infinitely loop the animation.
    float modelAnimationTime = std::fmod(static_cast<float>(time), anim->mEnd - anim->mStart);

    // As animations may have a start delay, we add this here.
    modelAnimationTime += anim->mStart;

    mModel->setAnimationTime(animation, modelAnimationTime);
  }

  // Then we update the uniform buffer data of all joint matrices for each skin.
  for (auto const& skin : mModel->getSkins()) {

    SkinUniforms skinBuffer{};
    auto         jointMatrices = skin->getJointMatrices();

    for (size_t i(0); i < jointMatrices.size() && i < 256; ++i) {
      skinBuffer.mJointMatrices[i] = jointMatrices[i];
    }

    // Create a new uniform buffer if there is none for the current skin. This should only happen in
    // the first few frames.
    if (mSkinBuffers.current().find(skin) == mSkinBuffers.current().end()) {
      mSkinBuffers.current()[skin] = Illusion::Graphics::CoherentBuffer::create("SkinUniformBuffer",
          mDevice, sizeof(SkinUniforms), vk::BufferUsageFlagBits::eUniformBuffer);
    }

    // Finally upload the data.
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

  // All data of the glTF model is stored in one big vertex buffer object.
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

    // Bind a uniform buffer for the skin data.
    if (n->mSkin) {
      auto buffer = mSkinBuffers.current()[n->mSkin]->getBuffer();
      cmd->bindingState().setUniformBuffer(buffer, sizeof(SkinUniforms), 0, 2, 0);
    } else {
      cmd->bindingState().setUniformBuffer(mEmptySkinBuffer->getBuffer(), 1, 0, 2, 0);
    }

    // The GltfShader uses four descriptor sets:
    // 0: Camera information (set by the main.cpp)
    // 1: BRDF textures (BRDFLuT + filtered environment textures, also set by the main.cpp)
    // 2: Model information, in this case the joint matrices (set above)
    // 3: Material information, this is only textures since all other values are set via push
    //    constants (set below)
    if (n->mMesh) {

      for (auto const& p : n->mMesh->mPrimitives) {

        // Only draw nodes with / without alpha blending to ensure correct compositing order.
        if (p.mMaterial->mDoAlphaBlending == doAlphaBlending) {

          // Set most material properties as push constants.
          PushConstants pushConstants{};
          pushConstants.mModelMatrix                = mModelMatrix * n->mGlobalTransform;
          pushConstants.mAlbedoFactor               = p.mMaterial->mAlbedoFactor;
          pushConstants.mEmissiveFactor             = p.mMaterial->mEmissiveFactor;
          pushConstants.mSpecularGlossinessWorkflow = p.mMaterial->mSpecularGlossinessWorkflow;
          pushConstants.mMetallicRoughnessFactor    = p.mMaterial->mMetallicRoughnessFactor;
          pushConstants.mNormalScale                = p.mMaterial->mNormalScale;
          pushConstants.mOcclusionStrength          = p.mMaterial->mOcclusionStrength;
          pushConstants.mAlphaCutoff                = p.mMaterial->mAlphaCutoff;
          pushConstants.mVertexAttributes           = static_cast<int32_t>(p.mVertexAttributes);
          cmd->pushConstants(pushConstants);

          // Bind the textures. If a model did not provide a texture, Illusion will generate a
          // one-by-one pixel default texture.
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

    // Continue drawing recursively.
    drawNodes(cmd, n->mChildren, viewMatrix, doAlphaBlending);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
