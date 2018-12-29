////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CommandLineOptions.hpp"

#include "Logger.hpp"

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
  ILLUSION_MESSAGE << mDescription << std::endl;

  uint32_t maxNameLength = 0;

  for (auto const& option : mOptions) {
    uint32_t nameLength = 0;
    for (auto const& name : option.mNames) {
      nameLength += name.size() + 2;
    }

    maxNameLength = std::max(maxNameLength, nameLength);
  }

  for (auto const& option : mOptions) {

    std::string names;

    for (auto const& name : option.mNames) {
      names += name + ", ";
    }

    std::stringstream sstr;
    sstr << std::left << std::setw(maxNameLength) << names.substr(0, names.size() - 2);

    size_t currentSpacePos  = 0;
    size_t currentLineWidth = 0;
    while (currentSpacePos != std::string::npos) {
      size_t nextSpacePos = option.mHelp.find_first_of(' ', currentSpacePos + 1);
      sstr << option.mHelp.substr(currentSpacePos, nextSpacePos - currentSpacePos);
      currentLineWidth += nextSpacePos - currentSpacePos;
      currentSpacePos = nextSpacePos;

      if (currentLineWidth > 60) {
        ILLUSION_MESSAGE << sstr.str() << std::endl;
        sstr = std::stringstream();
        sstr << std::left << std::setw(maxNameLength - 1) << " ";
        currentLineWidth = 0;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandLineOptions::parse(int argc, char* argv[]) const {
  int i = 1;
  while (i < argc) {
    std::string name(argv[i]);
    std::string value;

    size_t equalPos = name.find('=');
    if (equalPos != std::string::npos) {
      value = name.substr(equalPos + 1);
      name  = name.substr(0, equalPos);
    }

    if (i + 1 < argc && argv[i + 1][0] != '-') {
      value = argv[i + 1];
      ++i;
    }

    bool foundOption = false;

    for (auto const& option : mOptions) {
      if (std::find(option.mNames.begin(), option.mNames.end(), name) != option.mNames.end()) {
        foundOption = true;

        if (std::holds_alternative<bool*>(option.mValue)) {
          if (value.size() > 0 && value != "true" && value != "false") {
            throw std::runtime_error(
                "Failed to parse command line arguments: Value for bool type option \"" + name +
                "\" must be empty, true or false!");
          }
          *std::get<bool*>(option.mValue) = (value != "false");
        } else if (value == "") {
          throw std::runtime_error(
              "Failed to parse command line arguments: Missing value for option \"" + name + "\"!");
        } else if (std::holds_alternative<std::string*>(option.mValue)) {
          *std::get<std::string*>(option.mValue) = value;
        } else {
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

    if (!foundOption) {
      ILLUSION_WARNING << "Ignoring unknown command line option \"" << name << "\"." << std::endl;
    }

    ++i;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core
