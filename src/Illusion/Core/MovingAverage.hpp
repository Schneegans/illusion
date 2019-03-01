////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_MOVING_AVERAGE_HPP
#define ILLUSION_CORE_MOVING_AVERAGE_HPP

#include <array>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This tiny moving-average class can be used to calculate the average of the last C samples of a //
// signal very efficiently. It can be parametrized with the signals type (double, float, ...) and //
// the window size C.                                                                             //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, size_t C>
class MovingAverage {
 public:
  // Adds a new sample of the signal to the moving average window.
  void add(T value) {
    if (mItems == C) {
      mSum -= mValues[mNextIndex];
    }

    mValues[mNextIndex] = value;
    mSum += value;
    mNextIndex = (mNextIndex + 1) % C;
    mItems     = std::min(mItems + 1, C);
  }

  // Returns the current average.
  T get() const {
    return mSum / mItems;
  }

 private:
  std::array<T, C> mValues;
  size_t           mNextIndex = 0;
  size_t           mItems     = 0;
  T                mSum{};
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_MOVING_AVERAGE_HPP
