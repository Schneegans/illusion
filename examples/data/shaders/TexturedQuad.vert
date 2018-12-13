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
vec2 inPositions[4] = vec2[](
    vec2(0.85, -0.85),
    vec2(-0.85, -0.85),
    vec2(0.85, 0.85),
    vec2(-0.85, 0.85)
);

vec2 inTexcoords[4] = vec2[](
    vec2(1, 0),
    vec2(0, 0),
    vec2(1, 1),
    vec2(0, 1)
);

// outputs
layout(location = 0) out vec2 vTexcoords;

// methods
void main() {
    vTexcoords = inTexcoords[gl_VertexIndex];
    gl_Position = vec4(inPositions[gl_VertexIndex], 0.0, 1.0);
}
