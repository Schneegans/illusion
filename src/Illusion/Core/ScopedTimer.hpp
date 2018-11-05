////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_SCOPED_TIMER_HPP
#define ILLUSION_CORE_SCOPED_TIMER_HPP

// ---------------------------------------------------------------------------------------- includes
#include <string>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Can be used to measure time taken by some part of code.                                        //
////////////////////////////////////////////////////////////////////////////////////////////////////

class ScopedTimer {
 public:
  // -------------------------------------------------------------------------------- public methods
  ScopedTimer(std::string const& name);
  virtual ~ScopedTimer();

 private:
  // ------------------------------------------------------------------------------- private methods
  double getNow();

  // ------------------------------------------------------------------------------- private members
  std::string mName;
  double      mStartTime;
};
} // namespace Illusion::Core

#endif // ILLUSION_CORE_SCOPED_TIMER_HPP
