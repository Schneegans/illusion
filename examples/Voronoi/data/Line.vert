////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

layout(constant_id = 0) const float cR = 0;
layout(constant_id = 1) const float cG = 0;
layout(constant_id = 2) const float cB = 0;

// inputs
layout(location = 0) in vec2 inPosition;

// The color is passed to the fragment shader for interpolation.
layout(location = 0) out vec3 color;

void main() {
  color       = vec3(cR, cG, cB);
  gl_Position = vec4(inPosition, 0.0, 1.0);
}
