////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
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
  Timer(bool autoStart = true);

  void   start();
  double reset();

  double getElapsed() const;
  bool   isRunning() const;

  static double getNow();

 private:
  double mStart = 1.0;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_TIMER_HPP
