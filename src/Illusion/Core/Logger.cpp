////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "Logger.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace Illusion::Core {

bool Logger::enableTrace       = false;
bool Logger::enableDebug       = true;
bool Logger::enableMessage     = true;
bool Logger::enableWarning     = true;
bool Logger::enableError       = true;
bool Logger::enableColorOutput = true;

// no colors on windows!
#if defined(_WIN32)
const std::string Logger::PRINT_RED       = "";
const std::string Logger::PRINT_GREEN     = "";
const std::string Logger::PRINT_YELLOW    = "";
const std::string Logger::PRINT_BLUE      = "";
const std::string Logger::PRINT_PURPLE    = "";
const std::string Logger::PRINT_TURQUOISE = "";

const std::string Logger::PRINT_RED_BOLD       = "";
const std::string Logger::PRINT_GREEN_BOLD     = "";
const std::string Logger::PRINT_YELLOW_BOLD    = "";
const std::string Logger::PRINT_BLUE_BOLD      = "";
const std::string Logger::PRINT_PURPLE_BOLD    = "";
const std::string Logger::PRINT_TURQUOISE_BOLD = "";

const std::string Logger::PRINT_BOLD  = "";
const std::string Logger::PRINT_RESET = "";
#else  // _WIN32
const std::string Logger::PRINT_RED       = "\x001b[0;31m";
const std::string Logger::PRINT_GREEN     = "\x001b[0;32m";
const std::string Logger::PRINT_YELLOW    = "\x001b[0;33m";
const std::string Logger::PRINT_BLUE      = "\x001b[0;34m";
const std::string Logger::PRINT_PURPLE    = "\x001b[0;35m";
const std::string Logger::PRINT_TURQUOISE = "\x001b[0;36m";

const std::string Logger::PRINT_RED_BOLD       = "\x001b[1;31m";
const std::string Logger::PRINT_GREEN_BOLD     = "\x001b[1;32m";
const std::string Logger::PRINT_YELLOW_BOLD    = "\x001b[1;33m";
const std::string Logger::PRINT_BLUE_BOLD      = "\x001b[1;34m";
const std::string Logger::PRINT_PURPLE_BOLD    = "\x001b[1;35m";
const std::string Logger::PRINT_TURQUOISE_BOLD = "\x001b[1;36m";

const std::string Logger::PRINT_BOLD  = "\x001b[1m";
const std::string Logger::PRINT_RESET = "\x001b[0m";
#endif // else _WIN32

namespace {

class NullBuffer : public std::streambuf {
 public:
  int overflow(int c) override {
    return c;
  }
};

NullBuffer   nullBuffer;
std::ostream devNull(&nullBuffer);

std::ostream& print(
    std::ostream& os, bool enable, std::string const& header, std::string const& color) {
  if (enable) {
    return os << (Logger::enableColorOutput ? color : "") << header
              << (Logger::enableColorOutput ? Logger::PRINT_RESET : "") << " ";
  }
  return devNull;
}
} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::trace(std::ostream& os) {
  return print(os, enableTrace, "[ILLUSION][T]", PRINT_TURQUOISE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::debug(std::ostream& os) {
  return print(os, enableDebug, "[ILLUSION][D]", PRINT_BLUE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::message(std::ostream& os) {
  return print(os, enableMessage, "[ILLUSION][M]", PRINT_GREEN);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::warning(std::ostream& os) {
  return print(os, enableWarning, "[ILLUSION][W]", PRINT_YELLOW);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::error(std::ostream& os) {
  return print(os, enableError, "[ILLUSION][E]", PRINT_RED);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::traceCreation(std::string const& object, std::string const& name, std::ostream& os) {
  if (enableTrace) {
    trace(os) << PRINT_GREEN << "[create] " << PRINT_RESET << std::left << std::setw(20) << object
              << ((!name.empty()) ? " (" + name + ")" : "") << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::traceDeletion(std::string const& object, std::string const& name, std::ostream& os) {
  if (enableTrace) {
    trace(os) << PRINT_RED << "[delete] " << PRINT_RESET << std::left << std::setw(20) << object
              << " (" + name + ")" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Illusion::Core
