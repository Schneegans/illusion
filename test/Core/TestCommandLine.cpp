////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/CommandLine.hpp>

#include <doctest.h>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::Color") {
  std::string oString    = "Default Value";
  int32_t     oInteger   = -1;
  uint32_t    oUnsigned  = 0;
  double      oDouble    = 0.0;
  float       oFloat     = 0.f;
  bool        oBool      = false;
  bool        oPrintHelp = false;

  Illusion::Core::CommandLine cmd("Description.");
  cmd.addArgument({"-s", "--string"}, &oString, "Description");
  cmd.addArgument({"-i", "--integer"}, &oInteger, "Description");
  cmd.addArgument({"-u", "--unsigned"}, &oUnsigned, "Description");
  cmd.addArgument({"-d", "--double"}, &oDouble, "Description");
  cmd.addArgument({"-f", "--float"}, &oFloat, "Description");
  cmd.addArgument({"-b", "--bool"}, &oBool, "Description");
  cmd.addArgument({"-h", "--help"}, &oPrintHelp, "Description");

  SUBCASE("Checking default values untouched") {
    std::vector<char*> args = {const_cast<char*>("foo"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oString == "Default Value");
    CHECK(oInteger == -1);
    CHECK(oUnsigned == 0);
    CHECK(oDouble == 0.0);
    CHECK(oFloat == 0.f);
    CHECK(oBool == false);
    CHECK(oPrintHelp == false);
  }

  SUBCASE("Checking setting values") {
    std::vector<char*> args = {const_cast<char*>("foo"), const_cast<char*>("-s"),
        const_cast<char*>("foo bar"), const_cast<char*>("-i"), const_cast<char*>("-42"),
        const_cast<char*>("-u"), const_cast<char*>("128"), const_cast<char*>("-d"),
        const_cast<char*>("-234.3"), const_cast<char*>("-f"), const_cast<char*>("256.7"),
        const_cast<char*>("-b"), const_cast<char*>("true"), const_cast<char*>("--help"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oString == "foo bar");
    CHECK(oInteger == -42);
    CHECK(oUnsigned == 128);
    CHECK(oDouble == -234.3);
    CHECK(oFloat == 256.7f);
    CHECK(oBool == true);
    CHECK(oPrintHelp == true);
  }
}

} // namespace Illusion::Core
