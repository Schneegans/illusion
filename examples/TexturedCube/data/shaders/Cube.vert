////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

// A little more sophisticated shader which takes vertex attributes, a uniform buffer and push
// constants as input. The normal matrix is calculated on-the-fly which is preformance-wise
// sub-optimal but leaves some space in the push constants for other data. In the GltfShader, for
// example, all material information is uploaded with push constants.
// The camera information (in this case only the projection matrix) is bound as uniform buffer to
// descriptor set 0. Descriptor set 1 is used for material data (see TexturedCube.frag)

// inputs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoords;

// uniforms
layout(binding = 0, set = 0) uniform CameraUniforms {
  mat4 mProjection;
}
cameraUniforms;

// push constants
layout(push_constant, std430) uniform PushConstants {
  mat4 mModelView;
}
pushConstants;

// outputs
layout(location = 0) out vec2 vTexcoords;
layout(location = 1) out vec3 vNormal;

// methods
void main() {
  vTexcoords  = inTexcoords;
  vNormal     = (inverse(transpose(pushConstants.mModelView)) * vec4(inNormal, 0.0)).xyz;
  gl_Position = cameraUniforms.mProjection * pushConstants.mModelView * vec4(inPosition, 1.0);
}
