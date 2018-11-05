////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_ANIMATED_PROPERTY_HPP
#define ILLUSION_CORE_ANIMATED_PROPERTY_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Property.hpp"

#include <glm/glm.hpp>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// A class for smooth value interpolation.
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class AnimationDirection { IN, OUT, IN_OUT, OUT_IN, LINEAR };
enum class AnimationLoop { NONE, REPEAT, TOGGLE };

// -------------------------------------------------------------------------------------------------
template <typename T>
class AnimatedProperty : public Property<T> {
 public:
  // ---------------------------------------------------------------------------------- public types

  // -------------------------------------------------------------------------------- public members
  AnimationDirection mDirection = AnimationDirection::IN_OUT;
  AnimationLoop      mLoop      = AnimationLoop::NONE;
  double             mDuration  = 0.0;
  double             mExponent  = 0.0;
  double             mDelay     = 0.0;

  // -------------------------------------------------------------------------------- public signals
  Signal<> onFinish;

  // -------------------------------------------------------------------------------- public methods
  AnimatedProperty()
    : Property<T>()
    , mStart()
    , mEnd() {}

  AnimatedProperty(T const& val)
    : Property<T>(val)
    , mStart(val)
    , mEnd(val) {}

  AnimatedProperty(
    T const&           start,
    T const&           end,
    double             dur   = 1.0,
    AnimationDirection dir   = AnimationDirection::IN_OUT,
    AnimationLoop      loop  = AnimationLoop::NONE,
    double             exp   = 0.0,
    double             delay = 0.0)
    : Property<T>(start)
    , mDirection(dir)
    , mLoop(loop)
    , mDuration(dur)
    , mExponent(exp)
    , mDelay(delay)
    , mStart(start)
    , mEnd(end) {}

  void set(T const& value, double dur, double del = 0.0) {
    mStart    = this->get();
    mEnd      = value;
    mDuration = dur;
    mState    = 0.0;
    mDelay    = del;
  }

  void set(T const& value) override {
    mStart    = value;
    mEnd      = value;
    mDuration = 0.0;
    mState    = -1.0;
    mDelay    = 0.0;
    Property<T>::set(value);
  }

  void update(double time) {
    if (mDuration == 0.0 && mState != -1.0) { mState = 1.0; }

    if (mState < 1 && mState >= 0.0) {
      if (mDelay > 0) {
        mDelay -= time;
      } else {
        mState += time / mDuration;

        switch (mDirection) {
        case AnimationDirection::LINEAR:
          Property<T>::set(updateLinear(mState, mStart, mEnd));
          break;
        case AnimationDirection::IN:
          Property<T>::set(updateEaseIn(mState, mStart, mEnd));
          break;
        case AnimationDirection::OUT:
          Property<T>::set(updateEaseOut(mState, mStart, mEnd));
          break;
        case AnimationDirection::IN_OUT:
          Property<T>::set(updateEaseInOut(mState, mStart, mEnd));
          break;
        case AnimationDirection::OUT_IN:
          Property<T>::set(updateEaseOutIn(mState, mStart, mEnd));
          break;
        }
      }
    } else if (mState != -1.0) {
      Property<T>::set(mEnd);
      mState = -1.0;
      onFinish.emit();

      if (mLoop == AnimationLoop::REPEAT) {
        Property<T>::set(mStart);
        set(mEnd, mDuration);

      } else if (mLoop == AnimationLoop::TOGGLE) {
        set(mStart, mDuration);
      }
    }
  }

  inline void restart() {
    Property<T>::set(mStart);
    set(mEnd, mDuration);
  }
  inline T const&              start() const { return mStart; }
  inline T const&              end() const { return mEnd; }
  virtual AnimatedProperty<T>& operator=(T const& rhs) override {
    set(rhs);
    return *this;
  }

 protected:
  // ------------------------------------------------------------------------------- private methods
  T updateLinear(T const& t, T const& s, double e) {
    return glm::clamp((T)(s + t * (e - s)), mStart, mEnd);
  }

  T updateEaseIn(T const& t, T const& s, double e) {
    return s + (t * t * ((mExponent + 1) * t - mExponent)) * (e - s);
  }

  T updateEaseOut(T const& t, T const& s, double e) {
    return s + ((t - 1) * (t - 1) * ((mExponent + 1) * (t - 1) + mExponent) + 1) * (e - s);
  }

  T updateEaseInOut(T const& t, T const& s, double e) {
    if (mState < 0.5f)
      return updateEaseIn(t * 2, s, e - (e - s) * 0.5f);
    else
      return updateEaseOut(t * 2 - 1, s + (e - s) * 0.5f, e);
  }

  T updateEaseOutIn(T const& t, T const& s, double e) {
    if (mState < 0.5f)
      return updateEaseOut(t * 2, s, e - (e - s) * 0.5f);
    else
      return updateEaseIn(t * 2 - 1, s + (e - s) * 0.5f, e);
  }

  // ------------------------------------------------------------------------------- private members
  T      mStart, mEnd;
  double mState = 0.0;
};

std::ostream& operator<<(std::ostream& os, AnimationDirection value);
std::istream& operator>>(std::istream& is, AnimationDirection& value);
std::ostream& operator<<(std::ostream& os, AnimationLoop value);
std::istream& operator>>(std::istream& is, AnimationLoop& value);

class AnimatedFloat : public AnimatedProperty<float> {
 public:
  // -------------------------------------------------------------------------------- public methods
  AnimatedFloat()
    : AnimatedProperty<float>(0.f) {}
  AnimatedFloat(float val)
    : AnimatedProperty<float>(val) {}
  AnimatedFloat(
    float const&       start,
    float const&       end,
    double             duration  = 1.0,
    AnimationDirection direction = AnimationDirection::IN_OUT,
    AnimationLoop      loop      = AnimationLoop::NONE,
    double             exponent  = 0.0,
    double             delay     = 0.0)
    : AnimatedProperty<float>(start, end, duration, direction, loop, exponent, delay) {}

  AnimatedFloat& operator=(AnimatedFloat const& other) {
    mDirection = other.mDirection;
    mLoop      = other.mLoop;
    mDuration  = other.mDuration;
    mExponent  = other.mExponent;
    mDelay     = other.mDelay;
    mStart     = other.mStart;
    mEnd       = other.mEnd;
    mState     = other.mState;

    Property<float>::set(other.get());

    return *this;
  }
};

class AnimatedDouble : public AnimatedProperty<double> {
 public:
  // -------------------------------------------------------------------------------- public methods
  AnimatedDouble()
    : AnimatedProperty<double>(0.0) {}
  AnimatedDouble(double val)
    : AnimatedProperty<double>(val) {}
  AnimatedDouble(
    double const&      start,
    double const&      end,
    double             duration  = 1.0,
    AnimationDirection direction = AnimationDirection::IN_OUT,
    AnimationLoop      loop      = AnimationLoop::NONE,
    double             exponent  = 0.0,
    double             delay     = 0.0)
    : AnimatedProperty<double>(start, end, duration, direction, loop, exponent, delay) {}

  AnimatedDouble& operator=(AnimatedDouble const& other) {
    mDirection = other.mDirection;
    mLoop      = other.mLoop;
    mDuration  = other.mDuration;
    mExponent  = other.mExponent;
    mDelay     = other.mDelay;
    mStart     = other.mStart;
    mEnd       = other.mEnd;
    mState     = other.mState;

    Property<double>::set(other.get());

    return *this;
  }
};
} // namespace Illusion::Core

#endif // ILLUSION_CORE_ANIMATED_PROPERTY_HPP
