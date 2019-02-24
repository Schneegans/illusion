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

  Property()
      : mValue() {
  }

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

// ---------------------------------------------------------------------------------------------- //
// --------------------------------------- Tests ------------------------------------------------ //
// ---------------------------------------------------------------------------------------------- //

#ifdef DOCTEST_LIBRARY_INCLUDED

TEST_CASE("Illusion::Core::Property") {
  Double pDouble;
  Float  pFloat;
  Int8   pInt8;
  Int16  pInt16;
  Int32  pInt32;
  Int64  pInt64;
  UInt8  pUInt8;
  UInt16 pUInt16;
  UInt32 pUInt32;
  UInt64 pUInt64;
  Bool   pBool;
  String pString;
  FVec2  pFVec2;
  FVec3  pFVec3;
  FVec4  pFVec4;
  DVec2  pDVec2;
  DVec3  pDVec3;
  DVec4  pDVec4;
  IVec2  pIVec2;
  IVec3  pIVec3;
  IVec4  pIVec4;
  UVec2  pUVec2;
  UVec3  pUVec3;
  UVec4  pUVec4;
  FMat3  pFMat3;
  FMat4  pFMat4;
  DMat3  pDMat3;
  DMat4  pDMat4;

  SUBCASE("Checking default constructors") {
    CHECK(pDouble.get() == 0.0);
    CHECK(pFloat.get() == 0.f);
    CHECK(pInt8.get() == 0);
    CHECK(pInt16.get() == 0);
    CHECK(pInt32.get() == 0);
    CHECK(pInt64.get() == 0);
    CHECK(pUInt8.get() == 0u);
    CHECK(pUInt16.get() == 0u);
    CHECK(pUInt32.get() == 0u);
    CHECK(pUInt64.get() == 0u);
    CHECK(pBool.get() == false);
    CHECK(pString.get() == "");
    CHECK(pFVec2.get() == glm::fvec2());
    CHECK(pFVec3.get() == glm::fvec3());
    CHECK(pFVec4.get() == glm::fvec4());
    CHECK(pDVec2.get() == glm::dvec2());
    CHECK(pDVec3.get() == glm::dvec3());
    CHECK(pDVec4.get() == glm::dvec4());
    CHECK(pIVec2.get() == glm::ivec2());
    CHECK(pIVec3.get() == glm::ivec3());
    CHECK(pIVec4.get() == glm::ivec4());
    CHECK(pUVec2.get() == glm::uvec2());
    CHECK(pUVec3.get() == glm::uvec3());
    CHECK(pUVec4.get() == glm::uvec4());
    CHECK(pFMat3.get() == glm::fmat3());
    CHECK(pFMat4.get() == glm::fmat4());
    CHECK(pDMat3.get() == glm::dmat3());
    CHECK(pDMat4.get() == glm::dmat4());
  }
}

#endif

} // namespace Illusion::Core

#endif // ILLUSION_CORE_PROPERTY_HPP
