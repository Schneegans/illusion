////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ScopedTimer.hpp"

#include "Logger.hpp"
#include <chrono>
#include <iostream>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////

ScopedTimer::ScopedTimer(std::string const& name)
    : mName(name)
    , mStartTime(getNow()) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ScopedTimer::~ScopedTimer() {
  ILLUSION_DEBUG << mName << ": " << getNow() - mStartTime << " ms " << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double ScopedTimer::getNow() {
  auto time{std::chrono::system_clock::now().time_since_epoch()};
  return std::chrono::duration_cast<std::chrono::microseconds>(time).count() * 0.001;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core
