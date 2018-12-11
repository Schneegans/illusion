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
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexcoords;

layout(binding = 0, set = 1) uniform sampler2D uBaseColorTexture;
layout(binding = 1, set = 1) uniform sampler2D mMetallicRoughnessTexture;
layout(binding = 2, set = 1) uniform sampler2D mNormalTexture;
layout(binding = 3, set = 1) uniform sampler2D uOcclusionTexture;
layout(binding = 4, set = 1) uniform sampler2D uEmissiveTexture;

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

layout(push_constant, std430) uniform PushConstants {
  mat4     mModelView;
  Material mMaterial;
} pushConstants;

// outputs
layout(location = 0) out vec4 outColor;

void main() {
  vec3 lightDir = vec3(0, 0, 1);
  vec3 normal = normalize(vNormal);

  vec3 light = vec3(max(0, dot(normal, lightDir)));

  outColor = texture(uBaseColorTexture, vTexcoords);
  outColor *= pushConstants.mMaterial.mBaseColorFactor;
  outColor.rgb *= light;
  outColor.rgb *= texture(uOcclusionTexture, vTexcoords).rgb;
  // outColor.rgb += texture(uEmissiveTexture, vTexcoords).rgb * pushConstants.mMaterial.mEmissiveFactor;
}
