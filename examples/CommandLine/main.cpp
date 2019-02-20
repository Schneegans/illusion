////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/CommandLine.hpp>
#include <Illusion/Core/Logger.hpp>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows how to use Illusion's simple command line parser.For each possible          //
// argument it stores a pointer to a variable. When the corresponding argument is set on the      //
// command line (given to the parse() method) the variable is set to the given value. If the      //
// option is not set, the variable is not touched. Hence it should be initialized to a default    //
// state.                                                                                         //
// For each argument, several names (aliases) can be defined. Thus, the same boolean could be set //
// via '--help' or '-h'. While not required, it is a good practice to precede the argument names  //
// with either '--' or '-'. Except for booleans, a value is expected to be given. Booleans are    //
// set to 'true' if no value is provided (that means they can be used as simple flags as in       //
// the '--help' case). Values can be given in two ways: Either the option name and the value      //
// should be separated by a space or by a '='. Here are some valid examples:                      //
// --string="Foo Bar"                                                                             //
// --string "Foo Bar"                                                                             //
// --help                                                                                         //
// --help=false                                                                                   //
// --help true                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  // This variables can be set via the command line.
  std::string oString    = "Default Value";
  int32_t     oInteger   = -1;
  uint32_t    oUnsigned  = 0;
  double      oDouble    = 0.0;
  float       oFloat     = 0.f;
  bool        oBool      = false;
  bool        oPrintHelp = false;

  // First configure all possible command line options.
  Illusion::Core::CommandLine args("A demonstration of Illusion's simple command line parser.");
  args.addArgument({"-s", "--string"}, &oString, "A optional string value");
  args.addArgument({"-i", "--integer"}, &oInteger, "A integer value");
  args.addArgument({"-u", "--unsigned"}, &oUnsigned, "A unsigned value");
  args.addArgument({"-d", "--double"}, &oDouble, "A float value");
  args.addArgument({"-f", "--float"}, &oFloat, "A double value");
  args.addArgument({"-b", "--bool"}, &oBool, "A bool value");
  args.addArgument({"-h", "--help"}, &oPrintHelp,
      "Print this help. This help message is actually so long that it requires a line break!");

  // The do the actual parsing.
  try {
    args.parse(argc, argv);
  } catch (std::runtime_error const& e) {
    Illusion::Core::Logger::error() << e.what() << std::endl;
    return -1;
  }

  // When mPrintHelp was set to true, we print a help message and exit.
  if (oPrintHelp) {
    args.printHelp();
    return 0;
  }

  // Print the resulting values.
  Illusion::Core::Logger::message() << "mString: " << oString << std::endl;
  Illusion::Core::Logger::message() << "mInteger: " << oInteger << std::endl;
  Illusion::Core::Logger::message() << "mUnsigned: " << oUnsigned << std::endl;
  std::cout.precision(std::numeric_limits<double>::max_digits10);
  Illusion::Core::Logger::message() << "mDouble: " << oDouble << std::endl;
  std::cout.precision(std::numeric_limits<float>::max_digits10);
  Illusion::Core::Logger::message() << "mFloat: " << oFloat << std::endl;
  Illusion::Core::Logger::message() << "mBool: " << oBool << std::endl;

  return 0;
}
