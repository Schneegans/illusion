////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// inputs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoords;
layout(location = 3) in vec4 inJoint;
layout(location = 4) in vec4 inWeight;

// uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
  vec4 mPosition;
  mat4 mViewMatrix;
  mat4 mProjectionMatrix;
}
camera;

layout(set = 2, binding = 0) uniform SkinUniforms {
  mat4 mJointMatrices[256];
}
skin;

// push constants
const int HAS_NORMALS   = 1 << 0;
const int HAS_TEXCOORDS = 1 << 1;
const int HAS_SKINS     = 1 << 2;

layout(push_constant, std430) uniform PushConstants {
  mat4  mModelMatrix;
  vec4  mAlbedoFactor;
  vec3  mEmissiveFactor;
  bool  mSpecularGlossinessWorkflow;
  vec3  mMetallicRoughnessFactor;
  float mNormalScale;
  float mOcclusionStrength;
  float mAlphaCutoff;
  int   mVertexAttributes;
}
pushConstants;

// outputs
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexcoords;

// methods
void main() {
  vTexcoords = inTexcoords;

  mat4 modelMatrix = pushConstants.mModelMatrix;

  if ((pushConstants.mVertexAttributes & HAS_SKINS) > 0) {
    mat4 skinMat = inWeight.x * skin.mJointMatrices[int(inJoint.x)] +
                   inWeight.y * skin.mJointMatrices[int(inJoint.y)] +
                   inWeight.z * skin.mJointMatrices[int(inJoint.z)] +
                   inWeight.w * skin.mJointMatrices[int(inJoint.w)];

    modelMatrix = modelMatrix * skinMat;
  }

  vPosition = (modelMatrix * vec4(inPosition, 1.0)).xyz;

  if ((pushConstants.mVertexAttributes & HAS_NORMALS) > 0) {
    vNormal = inverse(transpose(mat3(modelMatrix))) * inNormal;
  }

  gl_Position = camera.mProjectionMatrix * camera.mViewMatrix * vec4(vPosition, 1.0);
}
