////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CommandLineOptions.hpp"

#include "Logger.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////

CommandLineOptions::CommandLineOptions(std::string const& description)
    : mDescription(description) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandLineOptions::addOption(
    std::vector<std::string> const& optionNames, OptionValue value, std::string const& help) {

  mOptions.emplace_back(Option{optionNames, value, help});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandLineOptions::printHelp() const {

  // print the general description
  Logger::message() << mDescription << std::endl;

  // find the option with the longest combined name length (in order to align the help messages)
  uint32_t maxNameLength = 0;

  for (auto const& option : mOptions) {
    uint32_t nameLength = 0;
    for (auto const& name : option.mNames) {
      nameLength += static_cast<uint32_t>(name.size()) + 2;
    }

    maxNameLength = std::max(maxNameLength, nameLength);
  }

  // print each option
  for (auto const& option : mOptions) {

    std::string names;

    for (auto const& name : option.mNames) {
      names += name + ", ";
    }

    std::stringstream sstr;
    sstr << std::left << std::setw(maxNameLength) << names.substr(0, names.size() - 2);

    // Print the help for each option. This is a bit more involved since we do line wrapping for
    // long descriptions.
    size_t currentSpacePos  = 0;
    size_t currentLineWidth = 0;
    while (currentSpacePos != std::string::npos) {
      size_t nextSpacePos = option.mHelp.find_first_of(' ', currentSpacePos + 1);
      sstr << option.mHelp.substr(currentSpacePos, nextSpacePos - currentSpacePos);
      currentLineWidth += nextSpacePos - currentSpacePos;
      currentSpacePos = nextSpacePos;

      if (currentLineWidth > 60) {
        Logger::message() << sstr.str() << std::endl;
        sstr = std::stringstream();
        sstr << std::left << std::setw(maxNameLength - 1) << " ";
        currentLineWidth = 0;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandLineOptions::parse(int argc, char* argv[]) const {

  // skip the first argument (name of the program)
  int i = 1;
  while (i < argc) {

    // we assume that the entire argument is an option name
    std::string name(argv[i]);
    std::string value;
    bool        valueIsSeperate = false;

    // if there is an '=' in the name, the part after the '=' is actually the value
    size_t equalPos = name.find('=');
    if (equalPos != std::string::npos) {
      value = name.substr(equalPos + 1);
      name  = name.substr(0, equalPos);
    }
    // else the following argument is the value
    else if (i + 1 < argc) {
      value           = argv[i + 1];
      valueIsSeperate = true;
    }

    // search for an option with the provided name
    bool foundOption = false;

    for (auto const& option : mOptions) {
      if (Utils::contains(option.mNames, name)) {
        foundOption = true;

        // In the case of booleans, there must not be a value present. So if the value is neither
        // 'true' nor 'false' it is considered to be the next argument
        if (std::holds_alternative<bool*>(option.mValue)) {
          if (value.size() > 0 && value != "true" && value != "false") {
            valueIsSeperate = false;
          }
          *std::get<bool*>(option.mValue) = (value != "false");
        }
        // In all other cases there must be a value
        else if (value == "") {
          throw std::runtime_error(
              "Failed to parse command line arguments: Missing value for option \"" + name + "\"!");
        }
        // For a std::string, we take the entire value
        else if (std::holds_alternative<std::string*>(option.mValue)) {
          *std::get<std::string*>(option.mValue) = value;
        }
        // In all other cases we use a std::stringstream to convert the value
        else {
          std::visit(
              [&value](auto&& arg) {
                std::stringstream sstr(value);
                sstr >> *arg;
              },
              option.mValue);
        }

        break;
      }
    }

    // Print a warning if there was an unknown option
    if (!foundOption) {
      Logger::warning() << "Ignoring unknown command line option \"" << name << "\"." << std::endl;
    }

    if (foundOption && valueIsSeperate) {
      ++i;
    }

    ++i;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core
