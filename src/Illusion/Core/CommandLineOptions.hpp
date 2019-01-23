////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_COMMAND_LINE_OPTIONS_HPP
#define ILLUSION_CORE_COMMAND_LINE_OPTIONS_HPP

#include <string>
#include <variant>
#include <vector>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This class is a simple and effective class to parse command line arguments. For each possible  //
// option it stores a pointer to a variables. When the corresponding option is set on the         //
// command line (given to the parse() method) the variable is set to the given value. If the      //
// option is not set, the variable is not touched. Hence it should be initialized to a default    //
// state.                                                                                         //
// For each option, several option names (aliases) can be defined. Thus, the same boolean could   //
// be set via '--help' or '-h'.While not required, it is a good practice to precede the option    //
// names with either '--' or '-'. Except for booleans, a value is expected to be given. Booleans  //
// are set to 'true' if no value is provided (that means they can be used as simple flags as in   //
// the '--help' case). Values can be given in two ways: Either the option name and the value      //
// should be followed by a space or they should be separated by and '='. Here are some valid      //
// examples:                                                                                      //
// --string="Foo Bar"                                                                             //
// --string "Foo Bar"                                                                             //
// --verbose                                                                                      //
// --verbose=false                                                                                //
// --verbose true                                                                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////

class CommandLineOptions {
 public:
  // These are the possible variables the options may point to. Bool and std::string are handled in
  // a special way, all other values are parsed with a std::stringstream. This std::variant can be
  // easily extended if the stream operators are overloaded. If not, you have to add a special case
  // to the parse() method.
  typedef std::variant<int*, double*, float*, bool*, std::string*> OptionValue;

  // The description is printed as part of the help message.
  explicit CommandLineOptions(std::string const& description);

  // Adds a possible option. A typical call would be like this:
  // bool printHelp = false;
  // cmd.addOption({"--help", "-h"}, &printHelp, "Print this help message");
  // Then, after parse() has been called, printHelp will be true if the user provided the flag.
  void addOption(
      std::vector<std::string> const& optionNames, OptionValue value, std::string const& help);

  // Prints the description given to the constructor and the help for each option.
  void printHelp() const;

  // The command line arguments are traversed from start to end. That means, if an option is set
  // multiple times, the last will be the one which is finally used. This will throw a
  // std::runtime_error when a value is missing for a given option.
  void parse(int argc, char* argv[]) const;

 private:
  std::string mDescription;

  struct Option {
    std::vector<std::string> mNames;
    OptionValue              mValue;
    std::string              mHelp;
  };

  std::vector<Option> mOptions;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_COMMAND_LINE_OPTIONS_HPP
