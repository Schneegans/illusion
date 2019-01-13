////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// The cube is drawn with a texture and very basic lighting. The texture is bound to the second
// descriptor set, the first is used by camera data (see TexturedCube.vert).

// inputs
layout(location = 0) in vec2 vTexcoords;
layout(location = 1) in vec3 vNormal;

// uniforms
layout(binding = 0, set = 1) uniform sampler2D texSampler;

// outputs
layout(location = 0) out vec4 outColor;

// methods
void main() {
  vec3 normal   = normalize(vNormal);
  vec3 lightDir = vec3(0, 0, 1);
  vec3 light    = vec3(max(0, dot(normal, lightDir)));
  outColor      = texture(texSampler, vTexcoords) * vec4(light, 1);
}
