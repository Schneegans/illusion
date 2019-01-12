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

// This shader is used by the ShaderSandbox example. It demonstrates hot-reloading of shader files.
// Once the example is running, you may edit this file, save it to disc and it will be automatically
// reloaded.
// This works for changes made to this shader or for any changes made to shaders which are included
// via the #include "filename.glsl" directive.

// inputs
layout(location = 0) in vec2 vTexcoords;

// push constants
layout(push_constant, std430) uniform PushConstants {
  float iTime;
  float iAspectRatio;
}
pushConstants;

// outputs
layout(location = 0) out vec4 oFragColor;

// The shader code below is directly taken from https://www.shadertoy.com/view/Xd2GzR
// It is licensed under the MIT License (Copyright © 2013 Inigo Quilez)

// It is just an example - feel free to experiment!

vec4 dcAdd(vec4 a, vec4 b) {
  return a + b;
}

vec4 dcMul(vec4 a, vec4 b) {
  return vec4(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x,
      a.x * b.z + a.z * b.x - a.y * b.w - a.w * b.y, a.x * b.w + a.w * b.x + a.z * b.y + a.y * b.z);
}

vec4 dcSqr(vec4 a) {
  return vec4(a.x * a.x - a.y * a.y, 2.0 * a.x * a.y, 2.0 * (a.x * a.z - a.y * a.w),
      2.0 * (a.x * a.w + a.y * a.z));
}

void main() {
  vec2 p = (vTexcoords * 2 - 1) * vec2(pushConstants.iAspectRatio, 1.0);

  // animation
  float tz = 0.5 - 0.5 * cos(0.225 * pushConstants.iTime);
  float zo = pow(0.5, 13.0 * tz);

  vec4 c = vec4(vec2(-0.05, .6805) + p * zo, 1.0, 0.0);

  float m2 = 0.0;
  float co = 0.0;

  vec4 z = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < 256; i++) {
    if (m2 > 1024.0) {
      continue;
    }

    // Z -> ZÂ² + c
    z = dcAdd(dcSqr(z), c);

    m2 = dot(z.xy, z.xy);
    co += 1.0;
  }

  // distance
  // d(c) = |Z|Â·log|Z|/|Z'|
  float d = 0.0;
  if (co < 256.0) {
    d = sqrt(dot(z.xy, z.xy) / dot(z.zw, z.zw)) * log(dot(z.xy, z.xy));
  }

  // do some soft coloring based on distance
  d        = clamp(4.0 * d / zo, 0.0, 1.0);
  d        = pow(d, 0.25);
  vec3 col = vec3(d);

  oFragColor = vec4(col, 1.0);
}
