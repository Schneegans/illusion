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

struct Light {
  vec4 mPosition;
  vec4 mColor;
};

// uniforms
layout(binding = 0, set = 0) readonly buffer LightBuffer {
  Light lights[];
}
lightBuffer;

// push constants
layout(push_constant, std430) uniform PushConstants {
  mat4 mModelViewProjection;
}
pushConstants;

// outputs
layout(location = 0) out vec4 vColor;

// methods
void main() {
  vColor      = lightBuffer.lights[gl_InstanceIndex].mColor;
  gl_Position = pushConstants.mModelViewProjection *
                (lightBuffer.lights[gl_InstanceIndex].mPosition + 0.01 * vec4(inPosition, 1.0));
}
