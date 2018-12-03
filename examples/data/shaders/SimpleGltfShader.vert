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
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoords;
layout(location = 3) in vec4 inJoint;
layout(location = 4) in vec4 inWeight;

// uniforms ----------------------------------------------------------------------------------------
layout(binding = 0, set = 0) uniform CameraUniforms {
    mat4 mProjection; 
} camera;

// push constants ----------------------------------------------------------------------------------
layout(push_constant, std430) uniform PushConstants {
    mat4 mModelView;
} pushConstants;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexcoords;

// methods -----------------------------------------------------------------------------------------
void main() {
    outTexcoords = inTexcoords;
    outNormal = inNormal;
    // gl_Position = vec4(inPosition, 1.0);
    // gl_Position = uniforms.projection * vec4(inPosition + vec3(0, 0, -10), 1.0);
    gl_Position = camera.mProjection * pushConstants.mModelView * vec4(inPosition, 1.0);
}
