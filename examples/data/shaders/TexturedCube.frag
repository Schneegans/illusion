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
layout(location = 0) in vec2 texcoords;

// uniforms ----------------------------------------------------------------------------------------
layout(binding = 1) uniform sampler2D texSampler;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec4 outColor;

// methods -----------------------------------------------------------------------------------------
void main() {
    outColor = texture(texSampler, texcoords);
}
