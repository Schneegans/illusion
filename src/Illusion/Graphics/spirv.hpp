////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SPIRV_HPP
#define ILLUSION_GRAPHICS_SPIRV_HPP

#include "fwd.hpp"

namespace Illusion::Graphics::Spirv {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<uint32_t> fromGlsl(std::string const& glsl, vk::ShaderStageFlagBits stage);

} // namespace Illusion::Graphics::Spirv

#endif // ILLUSION_GRAPHICS_SPIRV_HPP
