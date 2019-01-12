////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// This vertex shader does not take any vertex attributes as input. It solely uses gl_VertexIndex to
// index into the following two arrays in order to get position and texture coordinate data.
const float2 inPositions[4] = {float2(1, -1), float2(-1, -1), float2(1, 1), float2(-1, 1)};
const float2 inTexcoords[4] = {float2(1, 0), float2(0, 0), float2(1, 1), float2(0, 1)};

// This is a HLSL shader, the input and output parameters are named as they are in the GLSL version
// (see Quad.vert). This allows basically the same code in the main() method.
void main(uint gl_VertexIndex    : SV_VertexID, 
          out float2 vTexcoords  : TEXCOORD0, 
          out float4 gl_Position : SV_POSITION) {

  vTexcoords  = inTexcoords[gl_VertexIndex];
  gl_Position = float4(inPositions[gl_VertexIndex], 0.0, 1.0);
}
