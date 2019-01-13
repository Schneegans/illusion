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

// This vertex shader does not take any vertex attributes as input. It solely uses gl_VertexIndex to
// index into the following two arrays in order to get position and color data.
vec2 inPositions[3] = vec2[](vec2(0.5, -0.5), vec2(-0.5, -0.5), vec2(0.0, 0.5));
vec3 inColors[3]    = vec3[](vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));

// outputs
layout(location = 0) out vec3 color;

void main() {
  color       = inColors[gl_VertexIndex];
  gl_Position = vec4(inPositions[gl_VertexIndex], 0.0, 1.0);
}
