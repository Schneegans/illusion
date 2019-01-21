////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ScopedTimer.hpp"

#include "Logger.hpp"
#include <chrono>
#include <iostream>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////

ScopedTimer::ScopedTimer(std::string const& name)
    : NamedObject(name)
    , mStartTime(getNow()) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ScopedTimer::~ScopedTimer() {
  Logger::message() << getName() << ": " << getNow() - mStartTime << " ms " << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double ScopedTimer::getNow() {
  auto time = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(time).count() * 0.001;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core
