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
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoords;

// uniforms
layout(binding = 0, set = 0) uniform CameraUniforms {
    mat4 mProjection; 
} cameraUniforms;

// push constants
layout(push_constant, std430) uniform PushConstants {
    mat4 mModelView;
} pushConstants;

// outputs
layout(location = 0) out vec2 vTexcoords;
layout(location = 1) out vec3 vNormal;

// methods
void main() {
    vTexcoords = inTexcoords;
    vNormal = (inverse(transpose(pushConstants.mModelView)) * vec4(inNormal, 0.0)).xyz;
    gl_Position = cameraUniforms.mProjection * pushConstants.mModelView * vec4(inPosition, 1.0);
}
