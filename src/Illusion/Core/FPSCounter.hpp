////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_FPS_COUNTER_HPP
#define ILLUSION_CORE_FPS_COUNTER_HPP

#include "Property.hpp"
#include "Timer.hpp"

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class FPSCounter {

 public:
  // This property contains the current frames per seconds.
  Float pFPS = 0.f;

  // Every t frames the Fps property is updated.
  explicit FPSCounter(unsigned t = 100, bool autoStart = true);

  // Call this after creation of this counter.
  void start();

  // Call this once a frame.
  void step();

  // returns the number of times step() has been called
  unsigned getFrameCount() const;

 private:
  unsigned mFrameCount = 0;
  unsigned mDelay      = 10;
  Timer    mTimer;
};
} // namespace Illusion::Core

#endif // ILLUSION_CORE_FPS_COUNTER_HPP
