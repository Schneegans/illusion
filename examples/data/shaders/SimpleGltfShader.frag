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
layout(set = 0, binding = 0) uniform CameraUniforms {
  vec4 mPosition;
  mat4 mViewMatrix; 
  mat4 mProjectionMatrix;
} camera;

layout(set = 1, binding = 0) uniform sampler2D   uBRDFLuT;
layout(set = 1, binding = 1) uniform samplerCube uPrefilteredIrradiance;
layout(set = 1, binding = 2) uniform samplerCube uPrefilteredReflection;

layout(set = 3, binding = 0) uniform sampler2D uAlbedoTexture;
layout(set = 3, binding = 1) uniform sampler2D mMetallicRoughnessTexture;
layout(set = 3, binding = 2) uniform sampler2D mNormalTexture;
layout(set = 3, binding = 3) uniform sampler2D uOcclusionTexture;
layout(set = 3, binding = 4) uniform sampler2D uEmissiveTexture;

// push constants
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

// Gets metallic factor from specular glossiness workflow inputs 
float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular) {
  float perceivedDiffuse = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g + 0.114 * diffuse.b * diffuse.b);
  float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g + 0.114 * specular.b * specular.b);
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
  vec4 albedo = sRGBtoLinear(texture(uAlbedoTexture, vTexcoords)) * pushConstants.mAlbedoFactor;

  if (albedo.a < pushConstants.mAlphaCutoff) {
    discard;
  }


  vec3 viewDir = normalize(camera.mPosition.xyz - vPosition);

  vec3 normal;

  if ((pushConstants.mVertexAttributes & HAS_NORMALS) > 0) {
    normal = normalize(vNormal);
  } else {
    normal = normalize(cross(dFdy(vPosition), dFdx(vPosition)));
  }
  
  if ((pushConstants.mVertexAttributes & HAS_TEXCOORDS) > 0) {
    vec3 tangentNormal = texture(mNormalTexture, vTexcoords).rgb * 2.0 - 1.0;
    tangentNormal.xy *= pushConstants.mNormalScale;
    if (dot(normal, viewDir) < 0) {
     normal *= -1;
     tangentNormal.y *= -1;
    } else {
      tangentNormal.x *= -1;
    }
    normal = perturbNormal(normal, tangentNormal, vPosition, vTexcoords);
  }

  float roughness = 1.0;
  float metallic = 1.0;

  if (pushConstants.mSpecularGlossinessWorkflow) {

    roughness = 1.0 - texture(mMetallicRoughnessTexture, vTexcoords).a;

    // Convert metallic value from specular glossiness inputs
    vec3 specular = sRGBtoLinear(texture(mMetallicRoughnessTexture, vTexcoords)).rgb;
    float maxSpecular = max(max(specular.r, specular.g), specular.b);
    metallic = convertMetallic(albedo.rgb, specular, maxSpecular);

    const float epsilon = 1e-6;
    vec3 baseColorDiffusePart = albedo.rgb * ((1.0 - maxSpecular) / (1 - 0.04) / max(1 - metallic, epsilon)) * pushConstants.mAlbedoFactor.rgb;
    vec3 baseColorSpecularPart = specular - (vec3(0.04) * (1 - metallic) * (1 / max(metallic, epsilon))) * pushConstants.mMetallicRoughnessFactor.rgb;
    albedo = vec4(mix(baseColorDiffusePart, baseColorSpecularPart, metallic * metallic), albedo.a);

  } else {
    vec3 metallicRoughness = texture(mMetallicRoughnessTexture, vTexcoords).rgb * pushConstants.mMetallicRoughnessFactor;
    roughness = clamp(metallicRoughness.g, 0.04, 1);
    metallic = clamp(metallicRoughness.b, 0, 1);
  }

  outColor.rgb = vec3(0.0);

  vec3 f0 = vec3(0.04); 
  vec3 diffuseColor = albedo.rgb * (vec3(1.0) - f0) * (1.0 - metallic);
  vec3 specularColor = mix(f0, albedo.rgb, metallic);

  // vec3 lightDir = normalize(vec3(0, 1, 1));
  // outColor.rgb += directLighting(lightDir, viewDir, normal, specularColor, albedo.rgb, metallic, roughness);

  outColor.rgb += imageBasedLighting(viewDir, normal, specularColor, albedo.rgb, metallic, roughness);
  
  float occlusion = mix(1.0, texture(uOcclusionTexture, vTexcoords).r, pushConstants.mOcclusionStrength);
  outColor.rgb *= occlusion;

  vec3 emissive = sRGBtoLinear(texture(uEmissiveTexture, vTexcoords)).rgb * pushConstants.mEmissiveFactor;
  outColor.rgb += emissive;

  // Tone mapping
  outColor.rgb = Uncharted2Tonemap(outColor.rgb * 2);
  outColor.rgb = outColor.rgb * (1.0 / Uncharted2Tonemap(vec3(11.2)));  

  // Gamma correction
  outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
  outColor.a = albedo.a;
}
