////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_LOGGER_HPP
#define ILLUSION_LOGGER_HPP

#include <iostream>
#include <string>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints beautiful messages to the console output. You can use the macros at the bottom of this  //
// file like this:                                                                                //
// Logger::message() << "hello world" << std::endl; //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Logger {

 public:
  // These are constant which can be used to modify the color of cout streams. For now, this only
  // works on linux.
  const static std::string PRINT_RED;
  const static std::string PRINT_GREEN;
  const static std::string PRINT_YELLOW;
  const static std::string PRINT_BLUE;
  const static std::string PRINT_PURPLE;
  const static std::string PRINT_TURQUOISE;
  const static std::string PRINT_RED_BOLD;
  const static std::string PRINT_GREEN_BOLD;
  const static std::string PRINT_YELLOW_BOLD;
  const static std::string PRINT_BLUE_BOLD;
  const static std::string PRINT_PURPLE_BOLD;
  const static std::string PRINT_TURQUOISE_BOLD;
  const static std::string PRINT_BOLD;
  const static std::string PRINT_RESET;

  // If any one of these is set to false, the corresponding messages are discarded.
  static bool enableTrace;
  static bool enableDebug;
  static bool enableMessage;
  static bool enableWarning;
  static bool enableError;
  static bool enableColorOutput;

  // Use these to print something.
  static std::ostream& trace(std::ostream& os = std::cout);
  static std::ostream& debug(std::ostream& os = std::cout);
  static std::ostream& message(std::ostream& os = std::cout);
  static std::ostream& warning(std::ostream& os = std::cout);
  static std::ostream& error(std::ostream& os = std::cout);

  // Use this to print beautiful and consistent object life-time notifications on tracing level.
  static void traceCreation(
      std::string const& object, std::string const& name, std::ostream& os = std::cout);
  static void traceDeletion(
      std::string const& object, std::string const& name, std::ostream& os = std::cout);
};

} // namespace Illusion::Core

#endif // ILLUSION_LOGGER_HPP
