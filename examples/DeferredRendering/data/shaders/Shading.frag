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
layout(constant_id = 0) const int cLightCount = 1;

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
  vec4  emit   = subpassLoad(inEmit);
  vec4  normal = subpassLoad(inNormal);
  float depth  = subpassLoad(inDepth).r;

  vec4 position = pushConstants.mInvViewProjection * vec4((vTexcoords * 2 - 1), depth, 1);
  position      = position / position.w;

  outHdr = vec4(0, 0, 0, 1);

  vec4 cameraPosition = pushConstants.mInvViewProjection * vec4(0, 0, 0, 1);
  cameraPosition /= cameraPosition.w;

  for (int i = 0; i < cLightCount; ++i) {
    vec3  lightDir = (lightBuffer.lights[i].mPosition - position).xyz;
    float dist     = length(lightDir);

    if (dist < 5) {
      vec3  viewDir = normalize((cameraPosition - position).xyz);
      float diffuse = 0.1 * max(0, dot(lightDir / dist, normal.xyz) / (dist * dist));
      float specular =
          2 * pow(max(0, dot(reflect(lightDir / dist, normal.xyz), -viewDir)), 50) / (dist * dist);

      outHdr.rgb += lightBuffer.lights[i].mColor.rgb * (diffuse + specular);
    }
  }

  outHdr.rgb *= albedo.rgb;
  outHdr.rgb += emit.rgb;

  // outHdr.rgb = normal.xyz;
  // outHdr.a = 1;
  // outHdr = vec4(vec3(pow(depth, 10)), 1);

  // if (depth == 1) {
  //   outHdr = vec4(1, 0, 0, 1);
  // }
}
