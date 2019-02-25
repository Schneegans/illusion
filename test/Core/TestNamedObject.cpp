////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/NamedObject.hpp>

#include <doctest.h>

namespace Illusion::Core {

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

} // namespace Illusion::Core
