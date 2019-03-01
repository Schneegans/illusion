////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_FPS_COUNTER_HPP
#define ILLUSION_CORE_FPS_COUNTER_HPP

#include "MovingAverage.hpp"
#include "Property.hpp"
#include "Timer.hpp"

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The FPSCounter can be used to measure the average frame time and frames per second of the last //
// C frames. Just call step() once each frame. Internally, a MovingAverage is used to calculate   //
// the values.                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <size_t C>
class FPSCounter {

 public:
  // This property contains the average frames per seconds of the last C frames.
  Double pFPS = 0.0;

  // This property contains the average frames time of the last C frames.
  Double pFrameTime = 0.0;

  // Call this once a frame.
  void step() {
    mAverage.add(mTimer.restart());
    pFrameTime = mAverage.get();
    pFPS       = 1.0 / pFrameTime.get();
  }

 private:
  MovingAverage<double, C> mAverage;
  Timer                    mTimer;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_FPS_COUNTER_HPP
