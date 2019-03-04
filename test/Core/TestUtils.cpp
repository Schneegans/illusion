////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Utils.hpp>

#include <doctest.h>

#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::Utils::enumCast") {
  enum class MyEnum : uint32_t { a = 0, b, c };
  CHECK(Utils::enumCast(MyEnum::c) == 2u);
}

TEST_CASE("Illusion::Core::Utils::contains") {
  SUBCASE("Checking vector") {
    std::vector<int> constainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    CHECK(Utils::contains(constainer, 5) == true);
    CHECK(Utils::contains(constainer, 0) == false);
  }
  SUBCASE("Checking set") {
    std::set<int> constainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    CHECK(Utils::contains(constainer, 5) == true);
    CHECK(Utils::contains(constainer, 0) == false);
  }
  SUBCASE("Checking list") {
    std::list<int> constainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    CHECK(Utils::contains(constainer, 5) == true);
    CHECK(Utils::contains(constainer, 0) == false);
  }
  SUBCASE("Checking map") {
    std::map<int, int> constainer = {
        {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}};
    CHECK(Utils::contains(constainer, 5) == true);
    CHECK(Utils::contains(constainer, 0) == false);
  }
  SUBCASE("Checking unordered_map") {
    std::unordered_map<int, int> constainer = {
        {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}};
    CHECK(Utils::contains(constainer, 5) == true);
    CHECK(Utils::contains(constainer, 0) == false);
  }
}

TEST_CASE("Illusion::Core::Utils::splitString") {
  std::string test1("1|23,456|7,,,");
  std::string test2("");

  auto result = Utils::splitString(test1, ',');
  REQUIRE(result.size() == 4);
  CHECK(result[0] == "1|23");
  CHECK(result[1] == "456|7");
  CHECK(result[2] == "");
  CHECK(result[3] == "");

  result = Utils::splitString(test1, '|');
  REQUIRE(result.size() == 3);
  CHECK(result[0] == "1");
  CHECK(result[1] == "23,456");
  CHECK(result[2] == "7,,,");

  result = Utils::splitString(test1, '-');
  REQUIRE(result.size() == 1);
  CHECK(result[0] == "1|23,456|7,,,");

  result = Utils::splitString(test2, '-');
  REQUIRE(result.size() == 1);
  CHECK(result[0] == "");
}

TEST_CASE("Illusion::Core::Utils::joinStrings") {
  auto result = Utils::joinStrings({"Foo", "Bar", "", ",.-+\""});
  CHECK(result == "FooBar,.-+\"");

  result = Utils::joinStrings({"Foo", "Bar", "", ",.-+\""}, "--");
  CHECK(result == "Foo--Bar----,.-+\"");

  result = Utils::joinStrings({"Foo", "Bar", "", ",.-+\""}, ".", " and ");
  CHECK(result == "Foo.Bar. and ,.-+\"");
}

TEST_CASE("Illusion::Core::Utils::replaceString") {
  std::string test("1|23,456|7,,,");

  SUBCASE("Replace nothing") {
    Utils::replaceString(test, "foo", "bar");
    CHECK(test == "1|23,456|7,,,");
  }

  SUBCASE("Replace something in the middle") {
    Utils::replaceString(test, "456", "-----");
    CHECK(test == "1|23,-----|7,,,");
  }

  SUBCASE("Replace something at the beginning") {
    Utils::replaceString(test, "1", "");
    CHECK(test == "|23,456|7,,,");
  }

  SUBCASE("Replace something at the end") {
    Utils::replaceString(test, "7,,,", ".");
    CHECK(test == "1|23,456|.");
  }

  SUBCASE("Replace multiple occurences") {
    Utils::replaceString(test, ",", "!!");
    CHECK(test == "1|23!!456|7!!!!!!");
  }
}

} // namespace Illusion::Core
