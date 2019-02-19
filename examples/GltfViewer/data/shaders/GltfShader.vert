////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// The Gltf::Model fills one huge vertex buffer object with vertex data which is then used by all
// primitives. Therefore all primitives use the same vertex layout, even if they actually do not
// have texture coordinates, for example. In order to know which vertex attributes are actually set,
// there is the mVertexAttributes member of the push constants. It is a bitmask describing which
// attributes actually contain useful data.

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoords;
layout(location = 3) in vec4 inJoint;
layout(location = 4) in vec4 inWeight;

// The GltfShader uses four descriptor sets:
// 0: Camera information
// 1: BRDF textures (BRDFLuT + filtered environment textures)
// 2: Model information, in this case the joint matrices
// 3: Material information, this is only textures since all other values are set via push constants

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

// These three bits are potentially set in the mVertexAttributes member of the push constants. Use
// them in order to know which vertex attributes are actually set.
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

// Texture coordinates, world space positions and normals are passed to the fragment shader.
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexcoords;

void main() {

  // Just forward the texture coordinates.
  vTexcoords = inTexcoords;

  // Compute the skin matrix and multiply it with the model matrix if the model is skinned.
  mat4 modelMatrix = pushConstants.mModelMatrix;

  if ((pushConstants.mVertexAttributes & HAS_SKINS) > 0) {
    mat4 skinMat = inWeight.x * skin.mJointMatrices[int(inJoint.x)] +
                   inWeight.y * skin.mJointMatrices[int(inJoint.y)] +
                   inWeight.z * skin.mJointMatrices[int(inJoint.z)] +
                   inWeight.w * skin.mJointMatrices[int(inJoint.w)];

    modelMatrix = modelMatrix * skinMat;
  }

  // Transform to world space.
  vPosition = (modelMatrix * vec4(inPosition, 1.0)).xyz;

  // If there are normals, transform the to world space as well.
  if ((pushConstants.mVertexAttributes & HAS_NORMALS) > 0) {
    vNormal = inverse(transpose(mat3(modelMatrix))) * inNormal;
  }

  // Transform to projection space.
  gl_Position = camera.mProjectionMatrix * camera.mViewMatrix * vec4(vPosition, 1.0);
}
