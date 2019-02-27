////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_ANIMATED_PROPERTY_HPP
#define ILLUSION_CORE_ANIMATED_PROPERTY_HPP

#include "Property.hpp"

#include <glm/glm.hpp>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A class for smooth value interpolation.                                                        //
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class AnimationDirection { eIn, eOut, eInOut, eOutIn, eLinear };
enum class AnimationLoop { eNone, eRepeat, eToggle };

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class AnimatedProperty : public Property<T> {
 public:
  AnimationDirection mDirection = AnimationDirection::eInOut;
  AnimationLoop      mLoop      = AnimationLoop::eNone;
  double             mDuration  = 0.0;
  double             mExponent  = 0.0;
  double             mDelay     = 0.0;

  Signal<> onFinish;

  AnimatedProperty() = default;

  AnimatedProperty(T const& val)
      : Property<T>(val)
      , mStart(val)
      , mEnd(val) {
  }

  AnimatedProperty(T const& start, T const& end, double dur = 1.0,
      AnimationDirection dir = AnimationDirection::eInOut,
      AnimationLoop loop = AnimationLoop::eNone, double exp = 0.0, double delay = 0.0)
      : Property<T>(start)
      , mDirection(dir)
      , mLoop(loop)
      , mDuration(dur)
      , mExponent(exp)
      , mDelay(delay)
      , mStart(start)
      , mEnd(end) {
  }

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
    if (mDuration == 0.0 && mState != -1.0) {
      mState = 1.0;
    }

    if (mState < 1 && mState >= 0.0) {
      if (mDelay > 0) {
        mDelay -= time;
      } else {
        mState += time / mDuration;

        switch (mDirection) {
          case AnimationDirection::eLinear:
            Property<T>::set(updateLinear(mState, mStart, mEnd));
            break;
          case AnimationDirection::eIn:
            Property<T>::set(updateEaseIn(mState, mStart, mEnd));
            break;
          case AnimationDirection::eOut:
            Property<T>::set(updateEaseOut(mState, mStart, mEnd));
            break;
          case AnimationDirection::eInOut:
            Property<T>::set(updateEaseInOut(mState, mStart, mEnd));
            break;
          case AnimationDirection::eOutIn:
            Property<T>::set(updateEaseOutIn(mState, mStart, mEnd));
            break;
        }
      }
    } else if (mState != -1.0) {
      Property<T>::set(mEnd);
      mState = -1.0;
      onFinish.emit();

      if (mLoop == AnimationLoop::eRepeat) {
        Property<T>::set(mStart);
        set(mEnd, mDuration);

      } else if (mLoop == AnimationLoop::eToggle) {
        set(mStart, mDuration);
      }
    }
  }

  inline void restart() {
    Property<T>::set(mStart);
    set(mEnd, mDuration);
  }
  inline T const& start() const {
    return mStart;
  }
  inline T const& end() const {
    return mEnd;
  }
  virtual AnimatedProperty<T>& operator=(T const& rhs) override {
    mDirection = rhs.mDirection;
    mLoop      = rhs.mLoop;
    mDuration  = rhs.mDuration;
    mExponent  = rhs.mExponent;
    mDelay     = rhs.mDelay;
    mStart     = rhs.mStart;
    mEnd       = rhs.mEnd;
    mState     = rhs.mState;

    set(rhs);
    return *this;
  }

 protected:
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

  T      mStart, mEnd;
  double mState = 0.0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, AnimationDirection value);
std::istream& operator>>(std::istream& is, AnimationDirection& value);
std::ostream& operator<<(std::ostream& os, AnimationLoop value);
std::istream& operator>>(std::istream& is, AnimationLoop& value);

} // namespace Illusion::Core

#endif // ILLUSION_CORE_ANIMATED_PROPERTY_HPP
