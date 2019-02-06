////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_PROPERTY_HPP
#define ILLUSION_CORE_PROPERTY_HPP

#include "Signal.hpp"

#include <glm/glm.hpp>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A Property encpsulates a value and may inform you on any changes                               //
// applied to this value.                                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class Property {

 public:
  typedef T value_type;

  Property() = default;

  Property(T const& val)
      : mValue(val) {
  }

  Property(T&& val)
      : mValue(std::move(val)) {
  }

  Property(Property<T> const& toCopy)
      : mValue(toCopy.mValue) {
  }

  Property(Property<T>&& toCopy)
      : mValue(std::move(toCopy.mValue)) {
  }

  virtual ~Property() = default;

  // Returns a Signal which is fired when the internal value will be changed.
  // The old value is passed as parameter.
  virtual Signal<T> const& beforeChange() const {
    return mBeforeChange;
  }

  // Returns a Signal which is fired when the internal value has been changed.
  // The new value is passed as parameter.
  virtual Signal<T> const& onChange() const {
    return mOnChange;
  }

  // Sets the Property to a new value. beforeChange() and onChange() will be emitted.
  virtual void set(T const& value) {
    if (value != mValue) {
      mBeforeChange.emit(value);
      mValue = value;
      mOnChange.emit(value);
    }
  }

  // Sets the Property to a new value. beforeChange() and onChange() will not be emitted
  void setWithNoEmit(T const& value) {
    mValue = value;
  }

  // Emits beforeChange() and onChange() even if the value did not change
  void touch() {
    mBeforeChange.emit(mValue);
    mOnChange.emit(mValue);
  }

  // Returns the internal value
  virtual T const& get() const {
    return mValue;
  }

  // Connects two Properties to each other. If the source's value is changed,
  // this' value will be changed as well
  virtual void connectFrom(Property<T> const& source) {
    disconnect();
    mConnection   = &source;
    mConnectionId = source.onChange().connect([this](T const& value) {
      set(value);
      return true;
    });
    set(source.get());
  }

  // If this Property is connected from another property, it will e disconnected
  virtual void disconnect() {
    if (mConnection) {
      mConnection->onChange().disconnect(mConnectionId);
      mConnectionId = -1;
      mConnection   = nullptr;
    }
  }

  // If there are any Properties connected to this Property,
  // they won't be notified of any further changes
  virtual void disconnectAuditors() {
    mOnChange.disconnectAll();
    mBeforeChange.disconnectAll();
  }

  // Assigns the value of another Property
  virtual Property<T>& operator=(Property<T> const& rhs) {
    set(rhs.mValue);
    return *this;
  }

  // Assigns a new value to this Property
  virtual Property<T>& operator=(T const& rhs) {
    set(rhs);
    return *this;
  }

  // Compares the values of two Properties
  bool operator==(Property<T> const& rhs) const {
    return Property<T>::get() == rhs.get();
  }
  bool operator!=(Property<T> const& rhs) const {
    return Property<T>::get() != rhs.get();
  }

  // Compares the values of the Property to another value
  bool operator==(T const& rhs) const {
    return Property<T>::get() == rhs;
  }
  bool operator!=(T const& rhs) const {
    return Property<T>::get() != rhs;
  }

  // Returns the value of this Property
  operator T() const {
    return Property<T>::get();
  }
  T const& operator()() const {
    return Property<T>::get();
  }

 private:
  T         mValue;
  Signal<T> mOnChange;
  Signal<T> mBeforeChange;

  Property<T> const* mConnection   = nullptr;
  uint32_t           mConnectionId = 0;
};

// stream operators
template <typename T>
std::ostream& operator<<(std::ostream& outStream, Property<T> const& val) {
  outStream << val.get();
  return outStream;
}

template <typename T>
std::istream& operator>>(std::istream& inStream, Property<T>& val) {
  T tmp;
  inStream >> tmp;
  val.set(tmp);
  return inStream;
}

// typedefs
typedef Property<double>      Double;
typedef Property<float>       Float;
typedef Property<int8_t>      Int8;
typedef Property<int16_t>     Int16;
typedef Property<int32_t>     Int32;
typedef Property<int64_t>     Int64;
typedef Property<uint8_t>     UInt8;
typedef Property<uint16_t>    UInt16;
typedef Property<uint32_t>    UInt32;
typedef Property<uint64_t>    UInt64;
typedef Property<bool>        Bool;
typedef Property<std::string> String;

typedef Property<glm::fvec2> FVec2;
typedef Property<glm::fvec3> FVec3;
typedef Property<glm::fvec4> FVec4;
typedef Property<glm::dvec2> DVec2;
typedef Property<glm::dvec3> DVec3;
typedef Property<glm::dvec4> DVec4;
typedef Property<glm::ivec2> IVec2;
typedef Property<glm::ivec3> IVec3;
typedef Property<glm::ivec4> IVec4;
typedef Property<glm::uvec2> UVec2;
typedef Property<glm::uvec3> UVec3;
typedef Property<glm::uvec4> UVec4;

typedef Property<glm::fmat3> FMat3;
typedef Property<glm::fmat4> FMat4;
typedef Property<glm::dmat3> DMat3;
typedef Property<glm::dmat4> DMat4;

} // namespace Illusion::Core

#endif // ILLUSION_CORE_PROPERTY_HPP
