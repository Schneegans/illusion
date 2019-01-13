////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
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
