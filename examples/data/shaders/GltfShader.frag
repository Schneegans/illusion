////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

#include "ToneMapping.glsl"

// Texture coordinates, world space positions and normals are provided by the vertex shader.
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexcoords;

// The GltfShader uses four descriptor sets:
// 0: Camera information
// 1: BRDF textures (BRDFLuT + filtered environment textures)
// 2: Model information, in this case the joint matrices
// 3: Material information, this is only textures since all other values are set via push constants

layout(set = 0, binding = 0) uniform CameraUniforms {
  vec4 mPosition;
  mat4 mViewMatrix;
  mat4 mProjectionMatrix;
}
camera;

// BRDF textures
layout(set = 1, binding = 0) uniform sampler2D uBRDFLuT;
layout(set = 1, binding = 1) uniform samplerCube uPrefilteredIrradiance;
layout(set = 1, binding = 2) uniform samplerCube uPrefilteredReflection;

// Material textures. These are always set, even if the Gltf::Model actually does not have a
// corresponding texture. In this case, a default 1x1 pixel texture will be used.
layout(set = 3, binding = 0) uniform sampler2D uAlbedoTexture;
layout(set = 3, binding = 1) uniform sampler2D mMetallicRoughnessTexture;
layout(set = 3, binding = 2) uniform sampler2D mNormalTexture;
layout(set = 3, binding = 3) uniform sampler2D uOcclusionTexture;
layout(set = 3, binding = 4) uniform sampler2D uEmissiveTexture;

// These three bits are potentially set in the mVertexAttributes member of the push constants. Use
// them in order to know which vertex attributes are actually set.
const int HAS_NORMALS   = 1 << 0;
const int HAS_TEXCOORDS = 1 << 1;
const int HAS_SKINS     = 1 << 2;

layout(push_constant, std430) uniform PushConstants {
  mat4  mModelMatrix;
  vec4  mAlbedoFactor;
  vec3  mEmissiveFactor;
  bool  mSpecularGlossinessWorkflow;
  vec3  mMetallicRoughnessFactor;
  float mNormalScale;
  float mOcclusionStrength;
  float mAlphaCutoff;
  int   mVertexAttributes;
}
pushConstants;

// outputs
layout(location = 0) out vec4 outColor;

const float M_PI = 3.141592653589793;

// This PBR shader is based on
// https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/pbr.frag
// However several parts have been restructured.

// Normal Distribution function
float D_GGX(float dotNH, float roughness) {
  float alpha  = roughness * roughness;
  float alpha2 = alpha * alpha;
  float denom  = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
  return (alpha2) / (M_PI * denom * denom);
}

// Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness) {
  float r  = (roughness + 1.0);
  float k  = (r * r) / 8.0;
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

// Lookup prefiltered reflection
vec3 prefilteredReflection(vec3 reflectionDir, float roughness) {
  float lod = roughness * textureQueryLevels(uPrefilteredReflection);
  return textureLod(uPrefilteredReflection, reflectionDir, lod).rgb;
}

// Combine prefiltered reflection and irradiance
vec3 imageBasedLighting(vec3 V, vec3 N, vec3 f0, vec3 albedo, float metallic, float roughness) {

  // diffuse contribution
  vec3 irradiance = texture(uPrefilteredIrradiance, N).rgb;
  vec3 diffuse    = irradiance * albedo;

  // specular contribution
  vec3 reflectionDir = -normalize(reflect(V, N));
  vec3 reflection    = prefilteredReflection(reflectionDir, roughness).rgb;
  vec3 fresnel       = F_SchlickR(max(dot(N, V), 0.0), f0, roughness);
  vec2 brdf          = texture(uBRDFLuT, vec2(max(dot(N, V), 0.0), roughness)).rg;
  vec3 specular      = reflection * (fresnel * brdf.x + brdf.y);

  return (1.0 - fresnel) * (1.0 - metallic) * diffuse + specular;
}

// Compute lighting contribution by a point light source
vec3 directLighting(vec3 L, vec3 V, vec3 N, vec3 f0, vec3 albedo, float metallic, float roughness) {
  vec3  H     = normalize(V + L);
  float dotNH = clamp(dot(N, H), 0.0, 1.0);
  float dotNV = clamp(dot(N, V), 0.0, 1.0);
  float dotNL = clamp(dot(N, L), 0.0, 1.0);

  vec3 lightColor = vec3(1.0);

  vec3 color = vec3(0.0);

  if (dotNL > 0.0) {
    float normalDistribution = D_GGX(dotNH, roughness);
    float shadowing          = G_SchlicksmithGGX(dotNL, dotNV, roughness);
    vec3  fresnel            = F_Schlick(dotNV, f0);
    vec3  spec = normalDistribution * fresnel * shadowing / (4.0 * dotNL * dotNV + 0.001);
    vec3  kD   = (vec3(1.0) - fresnel) * (1.0 - metallic);
    color += (kD * albedo / M_PI + spec) * dotNL;
  }

  return color;
}

// See http://www.thetenthplanet.de/archives/1180
vec3 perturbNormal(vec3 normal, vec3 tangentNormal, vec3 position, vec2 texcoords) {
  // get edge vectors of the pixel triangle
  vec3 dp1  = dFdx(position);
  vec3 dp2  = dFdy(position);
  vec2 duv1 = dFdx(texcoords);
  vec2 duv2 = dFdy(texcoords);

  // solve the linear system
  vec3 dp2perp = cross(dp2, normal);
  vec3 dp1perp = cross(normal, dp1);
  vec3 T       = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B       = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame
  float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  mat3  TBN    = mat3(T * invmax, B * invmax, normal);

  return normalize(TBN * tangentNormal);
}

// Gets metallic factor from specular glossiness workflow inputs
float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular) {
  float perceivedDiffuse  = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g +
                                0.114 * diffuse.b * diffuse.b);
  float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g +
                                 0.114 * specular.b * specular.b);
  if (perceivedSpecular < 0.04) {
    return 0.0;
  }
  float a = 0.04;
  float b = perceivedDiffuse * (1.0 - maxSpecular) / (1.0 - 0.04) + perceivedSpecular - 2.0 * 0.04;
  float c = 0.04 - perceivedSpecular;
  float D = max(b * b - 4.0 * a * c, 0.0);
  return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

void main() {
  // Get base color, converted to linear space
  vec4 albedo = sRGBtoLinear(texture(uAlbedoTexture, vTexcoords)) * pushConstants.mAlbedoFactor;

  // Discard if below alpha threshold. mAlphaCutoff will be set to zero if this feature is disabled.
  if (albedo.a < pushConstants.mAlphaCutoff) {
    discard;
  }

  // Compute viewing direction.
  vec3 viewDir = normalize(camera.mPosition.xyz - vPosition);

  // Compute surface normal. This is either provided as vertex attribute or computed via the local
  // derivation of the world space positions.
  vec3 normal;
  if ((pushConstants.mVertexAttributes & HAS_NORMALS) > 0) {
    normal = normalize(vNormal);
  } else {
    normal = normalize(cross(dFdy(vPosition), dFdx(vPosition)));
  }

  // If we have texture coordinates we can use the normal map to disturb this normal
  if ((pushConstants.mVertexAttributes & HAS_TEXCOORDS) > 0) {
    vec3 tangentNormal = texture(mNormalTexture, vTexcoords).rgb * 2.0 - 1.0;
    tangentNormal.xy *= pushConstants.mNormalScale;

    // Flip normal and tangent space for back faces
    if (dot(normal, viewDir) < 0) {
      normal *= -1;
      tangentNormal.y *= -1;
    } else {
      tangentNormal.x *= -1;
    }
    normal = perturbNormal(normal, tangentNormal, vPosition, vTexcoords);
  }

  // Compute the metallic and roughness values. They are either provided by the material
  // (metallic-roughness workflow) or have to be computed (specular-glossiness workflow).
  float roughness = 1.0;
  float metallic  = 1.0;

  if (pushConstants.mSpecularGlossinessWorkflow) {

    // Convert roughness value from specular glossiness inputs
    roughness = 1.0 - texture(mMetallicRoughnessTexture, vTexcoords).a;

    // Convert metallic value from specular glossiness inputs
    vec3  specular    = sRGBtoLinear(texture(mMetallicRoughnessTexture, vTexcoords)).rgb;
    float maxSpecular = max(max(specular.r, specular.g), specular.b);
    metallic          = convertMetallic(albedo.rgb, specular, maxSpecular);

    // Convert diffuse and specular part
    const float e = 1e-6;

    vec3 baseColorDiffuse = albedo.rgb * ((1.0 - maxSpecular) / (1 - 0.04) / max(1 - metallic, e)) *
                            pushConstants.mAlbedoFactor.rgb;

    vec3 baseColorSpecular = specular - (vec3(0.04) * (1 - metallic) * (1 / max(metallic, e))) *
                                            pushConstants.mMetallicRoughnessFactor.rgb;

    albedo = vec4(mix(baseColorDiffuse, baseColorSpecular, metallic * metallic), albedo.a);

  } else {
    vec3 metallicRoughness =
        texture(mMetallicRoughnessTexture, vTexcoords).rgb * pushConstants.mMetallicRoughnessFactor;
    roughness = clamp(metallicRoughness.g, 0.04, 1);
    metallic  = clamp(metallicRoughness.b, 0, 1);
  }

  // Now compute the final color
  outColor.rgb = vec3(0.0);

  vec3 f0 = mix(vec3(0.04), albedo.rgb, metallic);

  // Add direct lighting
  // vec3 lightDir = normalize(vec3(0, 1, 1));
  // outColor.rgb += directLighting(lightDir, viewDir, normal, f0, albedo.rgb, metallic,
  // roughness);

  // Add image based lighting
  outColor.rgb += imageBasedLighting(viewDir, normal, f0, albedo.rgb, metallic, roughness);

  // Apply occlusion
  outColor.rgb *=
      mix(1.0, texture(uOcclusionTexture, vTexcoords).r, pushConstants.mOcclusionStrength);

  // Add emissive color
  outColor.rgb +=
      sRGBtoLinear(texture(uEmissiveTexture, vTexcoords)).rgb * pushConstants.mEmissiveFactor;

  // Apply tone mapping
  outColor.rgb = Uncharted2Tonemap(outColor.rgb, 2);

  // Apply gamma correction
  outColor   = linearToSRGB(outColor);
  outColor.a = albedo.a;
}
