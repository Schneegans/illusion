////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// inputs
layout(location = 0) in vec3 inPosition;

// uniforms
layout(binding = 0, set = 0) readonly buffer CameraUniforms {
  mat4 mModelViewProjection;
}
cameraUniforms;

// push constants
layout(push_constant, std430) uniform PushConstants {
  vec4 mPosition;
  vec4 mColor;
}
pushConstants;

// methods
void main() {
  gl_Position = cameraUniforms.mModelViewProjection *
                (pushConstants.mPosition + 0.02 * vec4(inPosition, 1.0));
}
