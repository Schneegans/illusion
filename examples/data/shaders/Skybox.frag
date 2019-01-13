////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

#include "ToneMapping.glsl"

// inputs
layout(location = 0) in vec2 vTexcoords;

// uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
  vec4 mPosition;
  mat4 mViewMatrix;
  mat4 mProjectionMatrix;
}
camera;

layout(set = 1, binding = 0) uniform samplerCube texEnvironmentMap;

// outputs
layout(location = 0) out vec4 outColor;

void main() {

  // Un-project the current fragment's position to world space
  vec4 farPos =
      inverse(camera.mProjectionMatrix * camera.mViewMatrix) * vec4(vTexcoords * 2 - 1, -1, 1);
  farPos /= farPos.w;

  // Use the direction from the camera to the un-projected fragment position as lookup direction
  // into the input cubemap
  outColor = texture(texEnvironmentMap, normalize((farPos - camera.mPosition).xyz));

  // Tone mapping
  outColor.rgb = Uncharted2Tonemap(outColor.rgb, 2);

  // Gamma correction
  outColor = linearToSRGB(outColor);
}
