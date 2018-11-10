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
layout(location = 0) in vec2 inTexcoords;

// uniforms ----------------------------------------------------------------------------------------
layout(binding = 0, set = 1) uniform MaterialUniforms {
    vec3 color; 
} material;

layout(push_constant, std430) uniform PushConstants {
    mat4 modelView;
} pushConstants;

layout(binding = 1, set = 1) uniform sampler2D texAlbedo;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec4 outColor;

// methods -----------------------------------------------------------------------------------------
void main() {
    // outColor = vec4(material.color, 1.0);
    outColor = texture(texAlbedo, inTexcoords);
}
