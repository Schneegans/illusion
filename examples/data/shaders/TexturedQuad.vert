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
vec2 positions[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5)
);

// push constants ----------------------------------------------------------------------------------
layout(push_constant, std430) uniform PushConstants {
    vec2 pos; 
    float time;
} pushConstants;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec2 texcoords;

// methods -----------------------------------------------------------------------------------------
void main() {
    texcoords = positions[gl_VertexIndex] + 0.5;
    gl_Position = vec4(positions[gl_VertexIndex] + pushConstants.pos + vec2(0, sin(pushConstants.time)), 0.0, 1.0);
}
