////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// inputs
layout(location = 0) in vec4 vColor;

// outputs
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

// methods
void main() {
  outColor  = vColor;
  outNormal = vec4(1, 0, 0, 1);
}
