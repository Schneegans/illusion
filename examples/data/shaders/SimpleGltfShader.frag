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

// uniforms
layout(binding = 0, set = 1) uniform sampler2D   uBRDFLuT;
layout(binding = 1, set = 1) uniform samplerCube uPrefilteredIrradiance;
layout(binding = 2, set = 1) uniform samplerCube uPrefilteredReflection;

layout(binding = 0, set = 2) uniform sampler2D uAlbedoTexture;
layout(binding = 1, set = 2) uniform sampler2D mMetallicRoughnessTexture;
layout(binding = 2, set = 2) uniform sampler2D mNormalTexture;
layout(binding = 3, set = 2) uniform sampler2D uOcclusionTexture;
layout(binding = 4, set = 2) uniform sampler2D uEmissiveTexture;

layout(binding = 0, set = 0) uniform CameraUniforms {
  vec4 mPosition;
  mat4 mViewMatrix; 
  mat4 mProjectionMatrix;
} camera;

// push constants
struct Material {
  vec4  mAlbedoFactor;
  vec3  mEmissiveFactor;
  float mMetallicFactor;
  float mRoughnessFactor;
  float mNormalScale;
  float mOcclusionStrength;
  float mAlphaCutoff;
};

const int HasNormals   = 1 << 0; 
const int HasTexcoords = 1 << 1; 
const int HasSkins     = 1 << 2; 

layout(push_constant, std430) uniform PushConstants {
  mat4     mModelMatrix;
  Material mMaterial;
  int      mVertexAttributes; 
} pushConstants;

// outputs
layout(location = 0) out vec4 outColor;

const float M_PI = 3.141592653589793;

// pbr shader based on
// https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/pbr.frag

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

// Normal Distribution function
float D_GGX(float dotNH, float roughness) {
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
  return (alpha2)/(M_PI * denom*denom); 
}

// Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;
  float GL = dotNL / (dotNL * (1.0 - k) + k);
  float GV = dotNV / (dotNV * (1.0 - k) + k);
  return GL * GV;
}

// Fresnel function
vec3 F_Schlick(float cosTheta, vec3 f0) {
  return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 f0, float roughness) {
  return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 prefilteredReflection(vec3 reflectionDir, float roughness) {
  float lod = roughness * textureQueryLevels(uPrefilteredReflection);
  float lodf = floor(lod);
  float lodc = ceil(lod);
  vec3 a = textureLod(uPrefilteredReflection, reflectionDir, lodf).rgb;
  vec3 b = textureLod(uPrefilteredReflection, reflectionDir, lodc).rgb;
  return mix(a, b, lod - lodf);
}

vec3 imageBasedLighting(vec3 V, vec3 N, vec3 f0, vec3 albedo, float metallic, float roughness) {

  // diffuse contribution
  vec3 irradiance = texture(uPrefilteredIrradiance, N).rgb;
  vec3 diffuse = irradiance * albedo;

  // specular contribution
  vec3 reflectionDir = -normalize(reflect(V, N));
  vec3 reflection = prefilteredReflection(reflectionDir, roughness).rgb;  
  vec3 fresnel = F_SchlickR(max(dot(N, V), 0.0), f0, roughness);
  vec2 brdf = texture(uBRDFLuT, vec2(max(dot(N, V), 0.0), roughness)).rg;
  vec3 specular = reflection * (fresnel * brdf.x + brdf.y);

  return (1.0 - fresnel) * (1.0 - metallic) * diffuse + specular;
}

vec3 directLighting(vec3 L, vec3 V, vec3 N, vec3 f0, vec3 albedo, float metallic, float roughness) {
  vec3 H = normalize(V + L);
  float dotNH = clamp(dot(N, H), 0.0, 1.0);
  float dotNV = clamp(dot(N, V), 0.0, 1.0);
  float dotNL = clamp(dot(N, L), 0.0, 1.0);

  vec3 lightColor = vec3(1.0);

  vec3 color = vec3(0.0);

  if (dotNL > 0.0) {
    float normalDistribution = D_GGX(dotNH, roughness); 
    float shadowing = G_SchlicksmithGGX(dotNL, dotNV, roughness);
    vec3 fresnel = F_Schlick(dotNV, f0);    
    vec3 spec = normalDistribution * fresnel * shadowing / (4.0 * dotNL * dotNV + 0.001);    
    vec3 kD = (vec3(1.0) - fresnel) * (1.0 - metallic);     
    color += (kD * albedo / M_PI + spec) * dotNL;
  }

  return color;
}

// See http://www.thetenthplanet.de/archives/1180
vec3 perturbNormal(vec3 normal, vec3 tangentNormal, vec3 position, vec2 texcoords) {
  // get edge vectors of the pixel triangle
  vec3 dp1 = dFdx( position );
  vec3 dp2 = dFdy( position );
  vec2 duv1 = dFdx( texcoords );
  vec2 duv2 = dFdy( texcoords );

  // solve the linear system
  vec3 dp2perp = cross( dp2, normal );
  vec3 dp1perp = cross( normal, dp1 );
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame 
  float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
  mat3 TBN = mat3( T * invmax, B * invmax, normal );

  return normalize(TBN * tangentNormal);
}

vec4 sRGBtoLinear(vec4 color) {
  return vec4(pow(color.rgb, vec3(2.2)), color.a);
}

void main() {
  vec4 albedo = sRGBtoLinear(texture(uAlbedoTexture, vTexcoords)) * pushConstants.mMaterial.mAlbedoFactor;

  if (albedo.a < pushConstants.mMaterial.mAlphaCutoff) {
    discard;
  }

  vec3 emissive = sRGBtoLinear(texture(uEmissiveTexture, vTexcoords)).rgb * pushConstants.mMaterial.mEmissiveFactor;

  vec3 viewDir = normalize(camera.mPosition.xyz - vPosition);

  vec3 normal;

  if ((pushConstants.mVertexAttributes & HasNormals) > 0) {
    normal = normalize(vNormal);
  } else {
    normal = normalize(cross(dFdy(vPosition), dFdx(vPosition)));
  }
  
  if ((pushConstants.mVertexAttributes & HasTexcoords) > 0) {
    vec3 tangentNormal = texture(mNormalTexture, vTexcoords).rgb * 2.0 - 1.0;
    tangentNormal *= pushConstants.mMaterial.mNormalScale;
    if (dot(normal, viewDir) < 0) {
     normal *= -1;
     tangentNormal.y *= -1;
    } else {
      tangentNormal.x *= -1;
    }
    normal = perturbNormal(normal, tangentNormal, vPosition, vTexcoords);
  }

  float occlusion = mix(1.0, texture(uOcclusionTexture, vTexcoords).r, pushConstants.mMaterial.mOcclusionStrength);
  float roughness = texture(mMetallicRoughnessTexture, vTexcoords).g * pushConstants.mMaterial.mRoughnessFactor;
  roughness = clamp(roughness, 0.04, 1);
  float metallic  = texture(mMetallicRoughnessTexture, vTexcoords).b * pushConstants.mMaterial.mMetallicFactor;
  metallic = clamp(metallic, 0, 1);

  outColor.rgb = vec3(0.0);

  vec3 f0 = vec3(0.04); 
  vec3 diffuseColor = albedo.rgb * (vec3(1.0) - f0) * (1.0 - metallic);
  vec3 specularColor = mix(f0, albedo.rgb, metallic);

  // vec3 lightDir = normalize(vec3(0, 1, 1));
  // outColor.rgb += directLighting(lightDir, viewDir, normal, specularColor, albedo.rgb, metallic, roughness);

  outColor.rgb += imageBasedLighting(viewDir, normal, specularColor, albedo.rgb, metallic, roughness);
  
  outColor.rgb *= occlusion;
  outColor.rgb += emissive;

  // Tone mapping
  outColor.rgb = Uncharted2Tonemap(outColor.rgb * 2);
  outColor.rgb = outColor.rgb * (1.0 / Uncharted2Tonemap(vec3(11.2)));  

  // Gamma correction
  outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));

 outColor.a = albedo.a;
}
