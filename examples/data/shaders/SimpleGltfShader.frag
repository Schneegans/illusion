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

// inputs ------------------------------------------------------------------------------------------
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexcoords;

// uniforms ----------------------------------------------------------------------------------------
// layout(binding = 0, set = 1) uniform MaterialUniforms {
//     vec4  mBaseColorFactor;
//     vec3  mEmissiveFactor;
//     float mMetallicFactor;
//     float mRoughnessFactor;
//     float mNormalScale;
//     float mOcclusionStrength;
// } material;

// layout(binding = 1, set = 1) uniform sampler2D uBaseColorTexture;
// layout(binding = 2, set = 1) uniform sampler2D uMetallicRoughnessTexture;
// layout(binding = 3, set = 1) uniform sampler2D uNormalTexture;
// layout(binding = 4, set = 1) uniform sampler2D uOcclusionTexture;
// layout(binding = 5, set = 1) uniform sampler2D uEmissiveTexture;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec4 outColor;

// methods -----------------------------------------------------------------------------------------
void main() {
    // outColor = vec4(inTexcoords - vec2(0, 1), 0, 1);
    outColor = vec4(normalize(inNormal)*0.5+0.5, 1);
    // outColor = texture(uBaseColorTexture, inTexcoords);
}
