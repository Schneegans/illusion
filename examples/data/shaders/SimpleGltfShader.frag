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
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexcoords;

layout(binding = 0, set = 1) uniform sampler2D uBaseColorTexture;
layout(binding = 1, set = 1) uniform sampler2D mMetallicRoughnessTexture;
layout(binding = 2, set = 1) uniform sampler2D mNormalTexture;
layout(binding = 3, set = 1) uniform sampler2D uOcclusionTexture;
layout(binding = 4, set = 1) uniform sampler2D uEmissiveTexture;

// push constants
struct Material {
  vec4  mBaseColorFactor;
  vec3  mEmissiveFactor;
  float mMetallicFactor;
  float mRoughnessFactor;
  float mNormalScale;
  float mOcclusionStrength;
  float mAlphaCutoff;
};

layout(push_constant, std430) uniform PushConstants {
  mat4     mModelView;
  Material mMaterial;
} pushConstants;

// outputs
layout(location = 0) out vec4 outColor;

const float M_PI = 3.141592653589793;

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}


// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
  return (alpha2)/(M_PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;
  float GL = dotNL / (dotNL * (1.0 - k) + k);
  float GV = dotNV / (dotNV * (1.0 - k) + k);
  return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 C, float metallic, float roughness)
{
  // Precalculate vectors and dot products  
  vec3 H = normalize (V + L);
  float dotNH = clamp(dot(N, H), 0.0, 1.0);
  float dotNV = clamp(dot(N, V), 0.0, 1.0);
  float dotNL = clamp(dot(N, L), 0.0, 1.0);

  // Light color fixed
  vec3 lightColor = vec3(1.0);

  vec3 color = vec3(0.0);

  if (dotNL > 0.0) {
    // D = Normal distribution (Distribution of the microfacets)
    float D = D_GGX(dotNH, roughness); 
    // G = Geometric shadowing term (Microfacets shadowing)
    float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
    // F = Fresnel factor (Reflectance depending on angle of incidence)
    vec3 F = F_Schlick(dotNV, F0);    
    vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);    
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);     
    color += (kD * C / M_PI + spec) * dotNL;
  }

  return color;
}

// See http://www.thetenthplanet.de/archives/1180
vec3 getNormal()
{
  vec3 tangentNormal = texture(mNormalTexture, vTexcoords).rgb * 2.0 - 1.0;
  tangentNormal *= pushConstants.mMaterial.mNormalScale;

  vec3 q1 = dFdx(vPosition);
  vec3 q2 = dFdy(vPosition);
  vec2 st1 = dFdx(vTexcoords);
  vec2 st2 = dFdy(vTexcoords);

  vec3 N = normalize(vNormal);
  vec3 T = normalize(q1 * st2.t - q2 * st1.t);
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return normalize(TBN * tangentNormal);
}

void main() {

  vec4  baseColor = texture(uBaseColorTexture, vTexcoords) * pushConstants.mMaterial.mBaseColorFactor;

  if (baseColor.a < pushConstants.mMaterial.mAlphaCutoff) {
    discard;
  }

  vec3  emissive  = texture(uEmissiveTexture, vTexcoords).rgb * pushConstants.mMaterial.mEmissiveFactor;
  vec3  normal    = getNormal();
  float occlusion = mix(1.0, texture(uOcclusionTexture, vTexcoords).r, pushConstants.mMaterial.mOcclusionStrength);
  float metallic  = texture(mMetallicRoughnessTexture, vTexcoords).b * pushConstants.mMaterial.mMetallicFactor;
  float roughness = texture(mMetallicRoughnessTexture, vTexcoords).g * pushConstants.mMaterial.mRoughnessFactor;

  baseColor.rgb = pow(baseColor.rgb, vec3(2.2));
  emissive.rgb = pow(emissive.rgb, vec3(2.2));

  vec3 F0 = vec3(0.04); 
  F0 = mix(F0, baseColor.rgb, metallic);

  vec3 lightDir = normalize(vec3(0, 1, 1));
  vec3 viewDir = normalize(-vPosition);
  vec3 Lo = specularContribution(lightDir, viewDir, normal, F0, baseColor.rgb, metallic, roughness);

  Lo *= occlusion;
  Lo += emissive;

  // Tone mapping
  outColor.rgb = Uncharted2Tonemap(Lo * 10);
  outColor.rgb = outColor.rgb * (1.0 / Uncharted2Tonemap(vec3(11.2)));  

  // Gamma correction
  outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));

 // outColor.rgb = vec3(roughness);
 outColor.a = baseColor.a;
}
