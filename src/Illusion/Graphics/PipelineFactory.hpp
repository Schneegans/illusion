////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_FACTORY_HPP
#define ILLUSION_GRAPHICS_PIPELINE_FACTORY_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

#include "../Core/BitHash.hpp"

#include <map>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class PipelineFactory {

 public:
  // -------------------------------------------------------------------------------- public methods
  PipelineFactory(std::shared_ptr<Context> const& context);
  virtual ~PipelineFactory();

  vk::Pipeline const& getPipelineHandle(
    GraphicsState const& graphicsState, vk::RenderPass const& renderpass, uint32_t subPass);

  void clearCache();

 private:
  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Context>                               mContext;
  std::map<Core::BitHash, std::shared_ptr<vk::Pipeline>> mCache;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_FACTORY_HPP
