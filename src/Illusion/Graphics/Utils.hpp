////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_UTILS_HPP
#define ILLUSION_GRAPHICS_UTILS_HPP

#include "fwd.hpp"

namespace Illusion::Graphics::Utils {

// some helpers regarding vk::Format
bool    isColorFormat(vk::Format format);
bool    isDepthFormat(vk::Format format);
bool    isDepthOnlyFormat(vk::Format format);
bool    isDepthStencilFormat(vk::Format format);
uint8_t getByteCount(vk::Format format);

} // namespace Illusion::Graphics::Utils

#endif // ILLUSION_GRAPHICS_UTILS_HPP
