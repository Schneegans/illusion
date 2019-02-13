////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// Specialization constants. These are set during command buffer recording.
layout(constant_id = 0) const float cPosX = 0;
layout(constant_id = 1) const float cPosY = 0;

// According to spec this could be constant, but glslang does not yet support this.
// https://github.com/KhronosGroup/glslang/issues/1074
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
vec2 cOffset = vec2(cPosX, cPosY);

// This vertex shader does not take any vertex attributes as input. It solely uses gl_VertexIndex to
// index into the following two arrays in order to get position data.
vec2 inPositions[3] =
    vec2[](vec2(0.4, -0.4) + cOffset, vec2(-0.4, -0.4) + cOffset, vec2(0.0, 0.4) + cOffset);

void main() {
  gl_Position = vec4(inPositions[gl_VertexIndex], 0.0, 1.0);
}
