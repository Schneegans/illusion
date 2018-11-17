////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_FWD_HPP
#define ILLUSION_GRAPHICS_FWD_HPP

// ---------------------------------------------------------------------------------------- includes
#include <vulkan/vulkan.hpp>

namespace Illusion::Graphics {

struct BackedBuffer;
struct BackedImage;

class Context;
class DisplayPass;
class Engine;
class Framebuffer;
class GraphicsState;
class Material;
class PhysicalDevice;
class PipelineFactory;
class RenderPass;
class ShaderModule;
class ShaderProgram;
class ShaderReflection;
class Surface;
class Texture;
class Window;

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_FWD_HPP
