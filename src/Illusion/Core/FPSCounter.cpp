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
#include "FPSCounter.hpp"

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////

FPSCounter::FPSCounter(unsigned t, bool autoStart)
  : mDelay(t) {

  if (autoStart) { start(); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FPSCounter::start() { mTimer.start(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void FPSCounter::step() {
  if (++mFrameCount == mDelay) {
    pFPS = 1.f * mDelay / float(mTimer.getElapsed());
    mTimer.reset();
    mFrameCount = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned FPSCounter::getFrameCount() const { return mFrameCount; }

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Illusion::Core
