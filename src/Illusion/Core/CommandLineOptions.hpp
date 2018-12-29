////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_COMMAND_LINE_OPTIONS_HPP
#define ILLUSION_CORE_COMMAND_LINE_OPTIONS_HPP

#include <string>
#include <variant>
#include <vector>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CommandLineOptions {

 public:
  typedef std::variant<int*, double*, float*, bool*, std::string*> OptionValue;

  explicit CommandLineOptions(std::string const& description);

  void addOption(
      std::vector<std::string> const& optionNames, OptionValue value, std::string const& help);

  void printHelp() const;

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
