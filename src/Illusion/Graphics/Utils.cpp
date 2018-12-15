////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Utils.hpp"

namespace Illusion::Graphics::Utils {

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t getByteCount(vk::Format format) {
  switch (format) {
  case vk::Format::eR4G4UnormPack8:
    return 1;
  case vk::Format::eR4G4B4A4UnormPack16:
    return 2;
  case vk::Format::eB4G4R4A4UnormPack16:
    return 2;
  case vk::Format::eR5G6B5UnormPack16:
    return 2;
  case vk::Format::eB5G6R5UnormPack16:
    return 2;
  case vk::Format::eR5G5B5A1UnormPack16:
    return 2;
  case vk::Format::eB5G5R5A1UnormPack16:
    return 2;
  case vk::Format::eA1R5G5B5UnormPack16:
    return 2;
  case vk::Format::eR8Unorm:
    return 1;
  case vk::Format::eR8Snorm:
    return 1;
  case vk::Format::eR8Uscaled:
    return 1;
  case vk::Format::eR8Sscaled:
    return 1;
  case vk::Format::eR8Uint:
    return 1;
  case vk::Format::eR8Sint:
    return 1;
  case vk::Format::eR8Srgb:
    return 1;
  case vk::Format::eR8G8Unorm:
    return 2;
  case vk::Format::eR8G8Snorm:
    return 2;
  case vk::Format::eR8G8Uscaled:
    return 2;
  case vk::Format::eR8G8Sscaled:
    return 2;
  case vk::Format::eR8G8Uint:
    return 2;
  case vk::Format::eR8G8Sint:
    return 2;
  case vk::Format::eR8G8Srgb:
    return 2;
  case vk::Format::eR8G8B8Unorm:
    return 3;
  case vk::Format::eR8G8B8Snorm:
    return 3;
  case vk::Format::eR8G8B8Uscaled:
    return 3;
  case vk::Format::eR8G8B8Sscaled:
    return 3;
  case vk::Format::eR8G8B8Uint:
    return 3;
  case vk::Format::eR8G8B8Sint:
    return 3;
  case vk::Format::eR8G8B8Srgb:
    return 3;
  case vk::Format::eB8G8R8Unorm:
    return 3;
  case vk::Format::eB8G8R8Snorm:
    return 3;
  case vk::Format::eB8G8R8Uscaled:
    return 3;
  case vk::Format::eB8G8R8Sscaled:
    return 3;
  case vk::Format::eB8G8R8Uint:
    return 3;
  case vk::Format::eB8G8R8Sint:
    return 3;
  case vk::Format::eB8G8R8Srgb:
    return 3;
  case vk::Format::eR8G8B8A8Unorm:
    return 4;
  case vk::Format::eR8G8B8A8Snorm:
    return 4;
  case vk::Format::eR8G8B8A8Uscaled:
    return 4;
  case vk::Format::eR8G8B8A8Sscaled:
    return 4;
  case vk::Format::eR8G8B8A8Uint:
    return 4;
  case vk::Format::eR8G8B8A8Sint:
    return 4;
  case vk::Format::eR8G8B8A8Srgb:
    return 4;
  case vk::Format::eB8G8R8A8Unorm:
    return 4;
  case vk::Format::eB8G8R8A8Snorm:
    return 4;
  case vk::Format::eB8G8R8A8Uscaled:
    return 4;
  case vk::Format::eB8G8R8A8Sscaled:
    return 4;
  case vk::Format::eB8G8R8A8Uint:
    return 4;
  case vk::Format::eB8G8R8A8Sint:
    return 4;
  case vk::Format::eB8G8R8A8Srgb:
    return 4;
  case vk::Format::eA8B8G8R8UnormPack32:
    return 4;
  case vk::Format::eA8B8G8R8SnormPack32:
    return 4;
  case vk::Format::eA8B8G8R8UscaledPack32:
    return 4;
  case vk::Format::eA8B8G8R8SscaledPack32:
    return 4;
  case vk::Format::eA8B8G8R8UintPack32:
    return 4;
  case vk::Format::eA8B8G8R8SintPack32:
    return 4;
  case vk::Format::eA8B8G8R8SrgbPack32:
    return 4;
  case vk::Format::eA2R10G10B10UnormPack32:
    return 4;
  case vk::Format::eA2R10G10B10SnormPack32:
    return 4;
  case vk::Format::eA2R10G10B10UscaledPack32:
    return 4;
  case vk::Format::eA2R10G10B10SscaledPack32:
    return 4;
  case vk::Format::eA2R10G10B10UintPack32:
    return 4;
  case vk::Format::eA2R10G10B10SintPack32:
    return 4;
  case vk::Format::eA2B10G10R10UnormPack32:
    return 4;
  case vk::Format::eA2B10G10R10SnormPack32:
    return 4;
  case vk::Format::eA2B10G10R10UscaledPack32:
    return 4;
  case vk::Format::eA2B10G10R10SscaledPack32:
    return 4;
  case vk::Format::eA2B10G10R10UintPack32:
    return 4;
  case vk::Format::eA2B10G10R10SintPack32:
    return 4;
  case vk::Format::eR16Unorm:
    return 2;
  case vk::Format::eR16Snorm:
    return 2;
  case vk::Format::eR16Uscaled:
    return 2;
  case vk::Format::eR16Sscaled:
    return 2;
  case vk::Format::eR16Uint:
    return 2;
  case vk::Format::eR16Sint:
    return 2;
  case vk::Format::eR16Sfloat:
    return 2;
  case vk::Format::eR16G16Unorm:
    return 4;
  case vk::Format::eR16G16Snorm:
    return 4;
  case vk::Format::eR16G16Uscaled:
    return 4;
  case vk::Format::eR16G16Sscaled:
    return 4;
  case vk::Format::eR16G16Uint:
    return 4;
  case vk::Format::eR16G16Sint:
    return 4;
  case vk::Format::eR16G16Sfloat:
    return 4;
  case vk::Format::eR16G16B16Unorm:
    return 6;
  case vk::Format::eR16G16B16Snorm:
    return 6;
  case vk::Format::eR16G16B16Uscaled:
    return 6;
  case vk::Format::eR16G16B16Sscaled:
    return 6;
  case vk::Format::eR16G16B16Uint:
    return 6;
  case vk::Format::eR16G16B16Sint:
    return 6;
  case vk::Format::eR16G16B16Sfloat:
    return 6;
  case vk::Format::eR16G16B16A16Unorm:
    return 8;
  case vk::Format::eR16G16B16A16Snorm:
    return 8;
  case vk::Format::eR16G16B16A16Uscaled:
    return 8;
  case vk::Format::eR16G16B16A16Sscaled:
    return 8;
  case vk::Format::eR16G16B16A16Uint:
    return 8;
  case vk::Format::eR16G16B16A16Sint:
    return 8;
  case vk::Format::eR16G16B16A16Sfloat:
    return 8;
  case vk::Format::eR32Uint:
    return 4;
  case vk::Format::eR32Sint:
    return 4;
  case vk::Format::eR32Sfloat:
    return 4;
  case vk::Format::eR32G32Uint:
    return 8;
  case vk::Format::eR32G32Sint:
    return 8;
  case vk::Format::eR32G32Sfloat:
    return 8;
  case vk::Format::eR32G32B32Uint:
    return 12;
  case vk::Format::eR32G32B32Sint:
    return 12;
  case vk::Format::eR32G32B32Sfloat:
    return 12;
  case vk::Format::eR32G32B32A32Uint:
    return 16;
  case vk::Format::eR32G32B32A32Sint:
    return 16;
  case vk::Format::eR32G32B32A32Sfloat:
    return 16;
  case vk::Format::eR64Uint:
    return 8;
  case vk::Format::eR64Sint:
    return 8;
  case vk::Format::eR64Sfloat:
    return 8;
  case vk::Format::eR64G64Uint:
    return 16;
  case vk::Format::eR64G64Sint:
    return 16;
  case vk::Format::eR64G64Sfloat:
    return 16;
  case vk::Format::eR64G64B64Uint:
    return 24;
  case vk::Format::eR64G64B64Sint:
    return 24;
  case vk::Format::eR64G64B64Sfloat:
    return 24;
  case vk::Format::eR64G64B64A64Uint:
    return 32;
  case vk::Format::eR64G64B64A64Sint:
    return 32;
  case vk::Format::eR64G64B64A64Sfloat:
    return 32;
  case vk::Format::eB10G11R11UfloatPack32:
    return 4;
  case vk::Format::eE5B9G9R9UfloatPack32:
    return 4;
  case vk::Format::eD16Unorm:
    return 2;
  case vk::Format::eX8D24UnormPack32:
    return 4;
  case vk::Format::eD32Sfloat:
    return 4;
  case vk::Format::eS8Uint:
    return 1;
  case vk::Format::eD16UnormS8Uint:
    return 3;
  case vk::Format::eD24UnormS8Uint:
    return 4;
  case vk::Format::eD32SfloatS8Uint:
    return 5;
  case vk::Format::eUndefined:
  case vk::Format::eBc1RgbUnormBlock:
  case vk::Format::eBc1RgbSrgbBlock:
  case vk::Format::eBc1RgbaUnormBlock:
  case vk::Format::eBc1RgbaSrgbBlock:
  case vk::Format::eBc2UnormBlock:
  case vk::Format::eBc2SrgbBlock:
  case vk::Format::eBc3UnormBlock:
  case vk::Format::eBc3SrgbBlock:
  case vk::Format::eBc4UnormBlock:
  case vk::Format::eBc4SnormBlock:
  case vk::Format::eBc5UnormBlock:
  case vk::Format::eBc5SnormBlock:
  case vk::Format::eBc6HUfloatBlock:
  case vk::Format::eBc6HSfloatBlock:
  case vk::Format::eBc7UnormBlock:
  case vk::Format::eBc7SrgbBlock:
  case vk::Format::eEtc2R8G8B8UnormBlock:
  case vk::Format::eEtc2R8G8B8SrgbBlock:
  case vk::Format::eEtc2R8G8B8A1UnormBlock:
  case vk::Format::eEtc2R8G8B8A1SrgbBlock:
  case vk::Format::eEtc2R8G8B8A8UnormBlock:
  case vk::Format::eEtc2R8G8B8A8SrgbBlock:
  case vk::Format::eEacR11UnormBlock:
  case vk::Format::eEacR11SnormBlock:
  case vk::Format::eEacR11G11UnormBlock:
  case vk::Format::eEacR11G11SnormBlock:
  case vk::Format::eAstc4x4UnormBlock:
  case vk::Format::eAstc4x4SrgbBlock:
  case vk::Format::eAstc5x4UnormBlock:
  case vk::Format::eAstc5x4SrgbBlock:
  case vk::Format::eAstc5x5UnormBlock:
  case vk::Format::eAstc5x5SrgbBlock:
  case vk::Format::eAstc6x5UnormBlock:
  case vk::Format::eAstc6x5SrgbBlock:
  case vk::Format::eAstc6x6UnormBlock:
  case vk::Format::eAstc6x6SrgbBlock:
  case vk::Format::eAstc8x5UnormBlock:
  case vk::Format::eAstc8x5SrgbBlock:
  case vk::Format::eAstc8x6UnormBlock:
  case vk::Format::eAstc8x6SrgbBlock:
  case vk::Format::eAstc8x8UnormBlock:
  case vk::Format::eAstc8x8SrgbBlock:
  case vk::Format::eAstc10x5UnormBlock:
  case vk::Format::eAstc10x5SrgbBlock:
  case vk::Format::eAstc10x6UnormBlock:
  case vk::Format::eAstc10x6SrgbBlock:
  case vk::Format::eAstc10x8UnormBlock:
  case vk::Format::eAstc10x8SrgbBlock:
  case vk::Format::eAstc10x10UnormBlock:
  case vk::Format::eAstc10x10SrgbBlock:
  case vk::Format::eAstc12x10UnormBlock:
  case vk::Format::eAstc12x10SrgbBlock:
  case vk::Format::eAstc12x12UnormBlock:
  case vk::Format::eAstc12x12SrgbBlock:
  case vk::Format::eG8B8G8R8422Unorm:
  case vk::Format::eB8G8R8G8422Unorm:
  case vk::Format::eG8B8R83Plane420Unorm:
  case vk::Format::eG8B8R82Plane420Unorm:
  case vk::Format::eG8B8R83Plane422Unorm:
  case vk::Format::eG8B8R82Plane422Unorm:
  case vk::Format::eG8B8R83Plane444Unorm:
  case vk::Format::eR10X6UnormPack16:
  case vk::Format::eR10X6G10X6Unorm2Pack16:
  case vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16:
  case vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16:
  case vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16:
  case vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16:
  case vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16:
  case vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16:
  case vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16:
  case vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16:
  case vk::Format::eR12X4UnormPack16:
  case vk::Format::eR12X4G12X4Unorm2Pack16:
  case vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16:
  case vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16:
  case vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16:
  case vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16:
  case vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16:
  case vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16:
  case vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16:
  case vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16:
  case vk::Format::eG16B16G16R16422Unorm:
  case vk::Format::eB16G16R16G16422Unorm:
  case vk::Format::eG16B16R163Plane420Unorm:
  case vk::Format::eG16B16R162Plane420Unorm:
  case vk::Format::eG16B16R163Plane422Unorm:
  case vk::Format::eG16B16R162Plane422Unorm:
  case vk::Format::eG16B16R163Plane444Unorm:
  case vk::Format::ePvrtc12BppUnormBlockIMG:
  case vk::Format::ePvrtc14BppUnormBlockIMG:
  case vk::Format::ePvrtc22BppUnormBlockIMG:
  case vk::Format::ePvrtc24BppUnormBlockIMG:
  case vk::Format::ePvrtc12BppSrgbBlockIMG:
  case vk::Format::ePvrtc14BppSrgbBlockIMG:
  case vk::Format::ePvrtc22BppSrgbBlockIMG:
  case vk::Format::ePvrtc24BppSrgbBlockIMG:
    throw std::runtime_error(
      "Failed to get byte count for " + vk::to_string(format) + ": Not implemented!");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics::Utils
