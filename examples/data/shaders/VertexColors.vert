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
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

// uniforms ----------------------------------------------------------------------------------------
layout(binding = 1) uniform Uniforms {
    mat4 projection; 
} uniforms;

// push constants ----------------------------------------------------------------------------------
layout(push_constant, std430) uniform PushConstants {
    mat4 modelView; 
} pushConstants;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec3 outColor;

// methods -----------------------------------------------------------------------------------------
void main() {
    outColor = inColor;
    gl_Position = uniforms.projection * pushConstants.modelView * vec4(inPosition, 1.0);
}
