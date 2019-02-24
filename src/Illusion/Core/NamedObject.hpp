////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_NAMED_OBJECT_HPP
#define ILLUSION_CORE_NAMED_OBJECT_HPP

#include <string>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The NamedObject is a tiny base class for all types which carry a human-readable name. This may //
// be very useful for debugging purposes. Usually the name is supposed to be immutable, but in    //
// order to support default-constructed derived classes, there is a default constructor. When     //
// this is used, derived classes should call setName() as soon as possible.                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

class NamedObject {
 public:
  NamedObject(std::string name = "Unamed Object");

  // Returns the name of the NamedObject.
  std::string const& getName() const;

 protected:
  void setName(std::string const& name);

 private:
  std::string mName;
};

// ---------------------------------------------------------------------------------------------- //
// --------------------------------------- Tests ------------------------------------------------ //
// ---------------------------------------------------------------------------------------------- //

#ifdef DOCTEST_LIBRARY_INCLUDED

TEST_CASE("Illusion::Core::NamedObject") {

  SUBCASE("Checking default constructor") {
    NamedObject object;
    CHECK(object.getName() == "Unamed Object");
  }

  SUBCASE("Checking non-default constructor") {
    NamedObject object("Foo Bar");
    CHECK(object.getName() == "Foo Bar");
  }

  SUBCASE("Checking setName()") {
    class Derived : public NamedObject {
     public:
      Derived() {
        setName("Foo Bar");
      }
    };
    Derived object;
    CHECK(object.getName() == "Foo Bar");
  }
}

#endif

} // namespace Illusion::Core

#endif // ILLUSION_CORE_NAMED_OBJECT_HPP
