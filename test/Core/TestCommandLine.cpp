////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/CommandLine.hpp>
#include <Illusion/Core/Logger.hpp>

#include <doctest.h>
#include <sstream>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::CommandLine") {
  std::string oString    = "Default Value";
  int32_t     oInteger   = -1;
  uint32_t    oUnsigned  = 0;
  double      oDouble    = 0.0;
  float       oFloat     = 0.f;
  bool        oBool      = false;
  bool        oPrintHelp = false;

  CommandLine cmd("Program description.");
  cmd.addArgument({"-s", "--string"}, &oString, "String description");
  cmd.addArgument({"-i", "--integer"}, &oInteger, "Integer description");
  cmd.addArgument({"-u", "--unsigned"}, &oUnsigned, "Unsigned description");
  cmd.addArgument({"-d", "--double"}, &oDouble, "Double description");
  cmd.addArgument({"-f", "--float"}, &oFloat, "Float description");
  cmd.addArgument({"-b", "--bool"}, &oBool, "Bool description");
  cmd.addArgument({"-h", "--help"}, &oPrintHelp, "PrintHelp description");

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

  SUBCASE("Checking passing argument with -i 1") {
    std::vector<char*> args = {
        const_cast<char*>("foo"), const_cast<char*>("-i"), const_cast<char*>("1"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oInteger == 1);
  }

  SUBCASE("Checking passing argument with --integer 1") {
    std::vector<char*> args = {
        const_cast<char*>("foo"), const_cast<char*>("--integer"), const_cast<char*>("1"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oInteger == 1);
  }

  SUBCASE("Checking passing argument with -i=1") {
    std::vector<char*> args = {const_cast<char*>("foo"), const_cast<char*>("-i=1"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oInteger == 1);
  }

  SUBCASE("Checking passing argument with --integer=1") {
    std::vector<char*> args = {const_cast<char*>("foo"), const_cast<char*>("--integer=1"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oInteger == 1);
  }

  SUBCASE("Checking passing no argument for bools") {
    std::vector<char*> args = {const_cast<char*>("foo"), const_cast<char*>("-b"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oBool == true);
  }

  SUBCASE("Checking passing false argument for bools") {
    oBool                   = true;
    std::vector<char*> args = {const_cast<char*>("foo"), const_cast<char*>("-b=false"), nullptr};
    cmd.parse(args.size() - 1, args.data());

    CHECK(oBool == false);
  }

  SUBCASE("Checking help output") {
    Logger::enableColorOutput = false;
    std::ostringstream oss;
    cmd.printHelp(oss);

    CHECK(oss);
    CHECK(oss.str() == R"([ILLUSION][M] Program description.
[ILLUSION][M] -s, --string    String description
[ILLUSION][M] -i, --integer   Integer description
[ILLUSION][M] -u, --unsigned  Unsigned description
[ILLUSION][M] -d, --double    Double description
[ILLUSION][M] -f, --float     Float description
[ILLUSION][M] -b, --bool      Bool description
[ILLUSION][M] -h, --help      PrintHelp description
)");

    Logger::enableColorOutput = true;
  }
}

} // namespace Illusion::Core
