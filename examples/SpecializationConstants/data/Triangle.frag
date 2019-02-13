////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// Specialization constants. These are set during command buffer recording.
layout(constant_id = 2) const float cRed   = 1;
layout(constant_id = 3) const float cGreen = 1;
layout(constant_id = 4) const float cBlue  = 1;

// The constant color is written to the framebuffer.
layout(location = 0) out vec4 outColor;

// In this very simple main method.
void main() {
  outColor = vec4(cRed, cGreen, cBlue, 1);
}
