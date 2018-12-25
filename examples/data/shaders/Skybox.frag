////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// inputs
layout(location = 0) in vec2 vTexcoords;

// uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
  vec4 mPosition;
  mat4 mViewMatrix; 
  mat4 mProjectionMatrix;
} camera;

layout(set = 1, binding = 0) uniform samplerCube texSampler;

// outputs
layout(location = 0) out vec4 outColor;

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main() {

  vec4 farPos = inverse(camera.mProjectionMatrix * camera.mViewMatrix) * vec4(vTexcoords*2-1, -1, 1);
  farPos /= farPos.w;

  outColor = texture(texSampler, normalize((farPos - camera.mPosition).xyz));

  // Tone mapping
  outColor.rgb = Uncharted2Tonemap(outColor.rgb * 2);
  outColor.rgb = outColor.rgb * (1.0 / Uncharted2Tonemap(vec3(11.2)));  
}
