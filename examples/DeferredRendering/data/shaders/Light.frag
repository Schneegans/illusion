////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// push constants
layout(push_constant, std430) uniform PushConstants {
  vec4 mPosition;
  vec4 mColor;
}
pushConstants;

// outputs
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

// methods
void main() {
  outColor  = pushConstants.mColor;
  outNormal = vec4(1, 0, 0, 1);
}
