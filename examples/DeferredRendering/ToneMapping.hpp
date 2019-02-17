////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_EXAMPLES_DEFERRED_RENDERING_TONEMAPPING_HPP
#define ILLUSION_EXAMPLES_DEFERRED_RENDERING_TONEMAPPING_HPP

#include <Illusion/Graphics/fwd.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ToneMapping {
 public:
  ToneMapping(Illusion::Graphics::DevicePtr const& device);

  void draw(Illusion::Graphics::CommandBufferPtr const&      cmd,
      std::vector<Illusion::Graphics::BackedImagePtr> const& inputAttachments);

 private:
  Illusion::Graphics::ShaderPtr mShader;
};

#endif // ILLUSION_EXAMPLES_DEFERRED_RENDERING_TONEMAPPING_HPP
