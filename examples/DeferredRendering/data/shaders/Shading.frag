////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// inputs
layout(location = 0) in vec2 vTexcoords;

// Specialization constants. These are set during command buffer recording.
layout(constant_id = 0) const float cLightCount = 1;

// uniforms
struct Light {
  vec4 mPosition;
  vec4 mColor;
};

layout(binding = 0, set = 1) readonly buffer LightBuffer {
  Light lights[];
}
lightBuffer;

// push constants
layout(push_constant, std430) uniform PushConstants {
  mat4 mInvViewProjection;
}
pushConstants;

// input attachments
layout(input_attachment_index = 0, binding = 0) uniform subpassInput inAlbedo;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inNormal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput inEmit;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput inDepth;

// outputs
layout(location = 0) out vec4 outHdr;

// methods
void main() {
  vec4  albedo = subpassLoad(inAlbedo);
  vec4  normal = subpassLoad(inNormal);
  vec4  emit   = subpassLoad(inEmit);
  float depth  = subpassLoad(inDepth).r;

  // vec4 position =
  //     pushConstants.mInvViewProjection * vec4((vTexcoords * 2 - 1) * vec2(1, 1), -depth, 1);

  // position = position / position.w;
  // outHdr   = position;
  outHdr   = albedo + emit;
  outHdr.a = 1;
  // outHdr = vec4(vec3(pow(depth, 10)), 1);

  // if (depth == 1) {
  //   outHdr = vec4(1, 0, 0, 1);
  // }
}
