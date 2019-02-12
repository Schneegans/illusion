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
vec3 inPositions[4] = vec3[](vec3(1, -1, -1), vec3(-1, -1, -1), vec3(1, -1, 1), vec3(-1, -1, 1));
vec2 inTexcoords[4] = vec2[](vec2(1, 0), vec2(0, 0), vec2(1, 1), vec2(0, 1));

// The texture coordinates are passed to the fragment shader.
layout(location = 0) out vec2 vTexcoords;

// push constants
layout(push_constant, std430) uniform PushConstants {
  mat4 mModelViewProjection;
}
pushConstants;

void main() {
  vTexcoords  = inTexcoords[gl_VertexIndex] * 10;
  gl_Position = pushConstants.mModelViewProjection *
                vec4(inPositions[gl_VertexIndex] * vec3(5.0, 1.0, 5.0), 1.0);
}
