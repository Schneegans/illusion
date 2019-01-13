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
  explicit Timer(bool autoStart = true);

  void   start();
  double reset();

  // In seconds.
  double getElapsed() const;

  bool isRunning() const;

  static double getNow();

 private:
  double mStart = 0.0;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_TIMER_HPP
