////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_TIMER_HPP
#define ILLUSION_CORE_TIMER_HPP

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A class which can be used to measure time intervals.                                           //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Timer {

 public:
  // The timer will start automatically, that means when getElapsed() is called for the first time,
  // it will report th time which passed since the construction of the timer.
  explicit Timer();

  // Returns the time which passed since the construction of the timer or since the last call to
  // restart(). In seconds.
  double getElapsed() const;

  // Returns the time which passed since the construction of the timer or since the last call to
  // restart(). In seconds.
  double restart();

  // Returns a timestamp as reported by std::chrono in seconds.
  static double getNow();

 private:
  double mStart = 0.0;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_TIMER_HPP
