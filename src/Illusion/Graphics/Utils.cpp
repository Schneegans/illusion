////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "Utils.hpp"

namespace Illusion::Graphics::Utils {

bool isColorFormat(vk::Format format) {
  return !isDepthStencilFormat(format) && !isDepthOnlyFormat(format);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool isDepthFormat(vk::Format format) {
  return isDepthStencilFormat(format) || isDepthOnlyFormat(format);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool isDepthOnlyFormat(vk::Format format) {
  return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool isDepthStencilFormat(vk::Format format) {
  return format == vk::Format::eD16UnormS8Uint || format == vk::Format::eD24UnormS8Uint ||
         format == vk::Format::eD32SfloatS8Uint;
}

} // namespace Illusion::Graphics::Utils
