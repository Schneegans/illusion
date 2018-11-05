////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "Logger.hpp"

#include <iostream>
#include <sstream>

namespace Illusion::Core {

bool Logger::printFile     = false;
bool Logger::printLine     = false;
bool Logger::enableTrace   = false;
bool Logger::enableDebug   = true;
bool Logger::enableMessage = true;
bool Logger::enableWarning = true;
bool Logger::enableError   = true;

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
  int overflow(int c) { return c; }
};

NullBuffer   nullBuffer;
std::ostream devNull(&nullBuffer);

std::string locationString(const char* f, int l) {
  if (!Logger::printFile && !Logger::printLine) { return ""; }

  std::stringstream sstr;

  sstr << "[";

  if (Logger::printFile) sstr << std::string(f);
  if (Logger::printFile && Logger::printLine) sstr << ":";
  if (Logger::printLine) sstr << l;

  sstr << "]";
  return sstr.str();
}

std::ostream& print(
  bool enable, std::string const& header, std::string const& color, const char* file, int line) {
  if (enable) {
    return std::cout << color << header << locationString(file, line) << Logger::PRINT_RESET << " ";
  } else {
    return devNull;
  }
}
} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::traceImpl(const char* file, int line) {
  return print(enableTrace, "[ILLUSION][T]", PRINT_TURQUOISE, file, line);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::debugImpl(const char* file, int line) {
  return print(enableDebug, "[ILLUSION][D]", PRINT_BLUE, file, line);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::messageImpl(const char* file, int line) {
  return print(enableMessage, "[ILLUSION][M]", PRINT_GREEN, file, line);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::warningImpl(const char* file, int line) {
  return print(enableWarning, "[ILLUSION][W]", PRINT_YELLOW, file, line);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Logger::errorImpl(const char* file, int line) {
  return print(enableError, "[ILLUSION][E]", PRINT_RED, file, line);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Illusion::Core
