////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// inputs
layout(location = 0) in vec2 vTexcoords;
layout(location = 1) in vec3 vPosition;

// uniforms
layout(binding = 0) uniform sampler2D texAlbedo;
layout(binding = 1) uniform sampler2D texNormal;

// outputs
layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outEmit;

// See http://www.thetenthplanet.de/archives/1180
vec3 perturbNormal(vec3 normal, vec3 tangentNormal, vec3 position, vec2 texcoords) {
  // get edge vectors of the pixel triangle
  vec3 dp1  = dFdx(position);
  vec3 dp2  = dFdy(position);
  vec2 duv1 = dFdx(texcoords);
  vec2 duv2 = dFdy(texcoords);

  // solve the linear system
  vec3 dp2perp = cross(dp2, normal);
  vec3 dp1perp = cross(normal, dp1);
  vec3 T       = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B       = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame
  float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  mat3  TBN    = mat3(T * invmax, B * invmax, normal);

  return normalize(TBN * tangentNormal);
}

// Very simple shader which just samples a texture. It is used in the TexturedQuad example, which
// can also load HLSL code. See Quad.ps for the HLSL version of this shader.
void main() {
  outNormal = vec4(perturbNormal(vec3(0, 1, 0), texture(texNormal, vTexcoords).rgb * 2 - 1,
                       vPosition, vTexcoords),
      1.0);
  outAlbedo = texture(texAlbedo, vTexcoords);
  outEmit   = vec4(0, 0, 0, 1);
}
