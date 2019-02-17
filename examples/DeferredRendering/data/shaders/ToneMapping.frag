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

// input attachments
layout(input_attachment_index = 0, binding = 0) uniform subpassInput inHdr;

// outputs
layout(location = 0) out vec4 outColor;

// Filmic tonemapping from http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Uncharted2Tonemap(vec3 color, float exposure) {
  vec3 result = Uncharted2Tonemap(color * exposure);
  result      = result * (1.0 / Uncharted2Tonemap(vec3(11.2)));
  return result;
}

// Very simplistic gamma correction
vec4 linearToSRGB(vec4 color) {
  return vec4(pow(color.rgb, vec3(1.0 / 2.2)), color.a);
}

vec4 sRGBtoLinear(vec4 color) {
  return vec4(pow(color.rgb, vec3(2.2)), color.a);
}

void main() {
  // const float exposure = 1;
  // outColor             = vec4(Uncharted2Tonemap(subpassLoad(inHdr).rgb, exposure), 1);
  // outColor             = linearToSRGB(outColor);
  outColor = subpassLoad(inHdr);
}
