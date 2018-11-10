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

// inputs ------------------------------------------------------------------------------------------
vec3 positions[14] = vec3[](
    vec3(0, 0, 0),
    vec3(0, 1, 0),
    vec3(1, 0, 0),
    vec3(1, 1, 0),
    vec3(1, 1, 1),
    vec3(0, 1, 0),
    vec3(0, 1, 1),
    vec3(0, 0, 1),
    vec3(1, 1, 1),
    vec3(1, 0, 1),
    vec3(1, 0, 0),
    vec3(0, 0, 1),
    vec3(0, 0, 0),
    vec3(0, 1, 0)
);

vec2 texcoords[14] = vec2[](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 1),
    vec2(1, 1),
    vec2(1, 1), 
    vec2(0, 1),
    vec2(0, 0),
    vec2(1, 1),
    vec2(1, 0),
    vec2(0, 1),
    vec2(0, 0),
    vec2(1, 0),
    vec2(1, 1)
);

// uniforms ----------------------------------------------------------------------------------------
layout(binding = 2) uniform Uniforms {
    mat4 projection; 
} uniforms;

// push constants ----------------------------------------------------------------------------------
layout(push_constant, std430) uniform PushConstants {
    mat4 modelView; 
} pushConstants;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec2 vTexcoords;

// methods -----------------------------------------------------------------------------------------
void main() {
    vTexcoords = texcoords[gl_VertexIndex];
    gl_Position = uniforms.projection * pushConstants.modelView * vec4((positions[gl_VertexIndex]-vec3(0.5)), 1.0);
}
