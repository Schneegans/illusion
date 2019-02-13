////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// The color is passed in from the vertex shader.
layout(location = 0) in vec3 color;

// And is directly written to the framebuffer.
layout(location = 0) out vec4 outColor;

// In this very simple main method.
void main() {
  outColor = vec4(color, 1);
}
