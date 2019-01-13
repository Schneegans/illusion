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
layout(location = 0) in vec2 vTexcoords;

// uniforms
layout(binding = 0) uniform sampler2D texSampler;

// outputs
layout(location = 0) out vec4 outColor;

// Very simple shader which just samples a texture. It is used in the TexturedQuad example, which
// can also load HLSL code. See TexturedQuad.ps for the HLSL version of this shader.
void main() {
  outColor = texture(texSampler, vTexcoords);
}
