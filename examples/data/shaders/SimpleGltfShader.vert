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
layout(binding = 0, set = 0) uniform CameraUniforms {
  vec4 mPosition;
  mat4 mViewMatrix; 
  mat4 mProjectionMatrix;
} camera;

// push constants
struct Material {
  vec4  mBaseColorFactor;
  vec3  mEmissiveFactor;
  float mMetallicFactor;
  float mRoughnessFactor;
  float mNormalScale;
  float mOcclusionStrength;
  float mAlphaCutoff;
};

const int HasNormals   = 1 << 0; 
const int HasTexcoords = 1 << 1; 
const int HasSkins     = 1 << 2; 

layout(push_constant, std430) uniform PushConstants {
  mat4     mModelMatrix;
  Material mMaterial;
  int      mVertexAttributes; 
} pushConstants;

// outputs
layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexcoords;

// methods
void main() {
  vTexcoords = inTexcoords;

  if ((pushConstants.mVertexAttributes & HasNormals) > 0) {
    vNormal = (inverse(transpose(pushConstants.mModelMatrix)) * vec4(inNormal, 0.0)).xyz;
  }

  vPosition = (pushConstants.mModelMatrix * vec4(inPosition, 1.0)).xyz;
  gl_Position = camera.mProjectionMatrix * camera.mViewMatrix * vec4(vPosition, 1.0);
}
