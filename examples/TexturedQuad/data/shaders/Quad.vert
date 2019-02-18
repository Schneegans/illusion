////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// This vertex shader does not take any vertex attributes as input. It solely uses gl_VertexIndex to
// index into the following two arrays in order to get position and texture coordinate data.
vec2 inPositions[3] = vec2[](vec2(-1, -3), vec2(-1, 1), vec2(3, 1));
vec2 inTexcoords[3] = vec2[](vec2(0, -1), vec2(0, 1), vec2(2, 1));

// The texture coordinates are passed to the fragment shader.
layout(location = 0) out vec2 vTexcoords;

void main() {
  vTexcoords  = inTexcoords[gl_VertexIndex];
  gl_Position = vec4(inPositions[gl_VertexIndex], 0.0, 1.0);
}
