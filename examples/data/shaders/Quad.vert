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
// index into the following two arrays in order to get position and texture coordinate data.
vec2 inPositions[4] = vec2[](vec2(1, -1), vec2(-1, -1), vec2(1, 1), vec2(-1, 1));
vec2 inTexcoords[4] = vec2[](vec2(1, 0), vec2(0, 0), vec2(1, 1), vec2(0, 1));

// The texture coordinates are passed to the fragment shader.
layout(location = 0) out vec2 vTexcoords;

void main() {
  vTexcoords  = inTexcoords[gl_VertexIndex];
  gl_Position = vec4(inPositions[gl_VertexIndex], 0.0, 1.0);
}
