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
layout(location = 1) in vec3 vNormal;

// uniforms
layout(binding = 0, set = 1) uniform sampler2D texSampler;

// outputs
layout(location = 0) out vec4 outColor;

// methods
void main() {
  vec3  lightDir = vec3(0, 0, 1);
  vec3  normal   = normalize(vNormal);
  float light    = max(0, dot(normal, lightDir));
  outColor       = texture(texSampler, vTexcoords) * vec4(light, light, light, 1);
}
