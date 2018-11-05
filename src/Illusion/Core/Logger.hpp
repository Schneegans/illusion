////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_LOGGER_HPP
#define ILLUSION_LOGGER_HPP

// ---------------------------------------------------------------------------------------- includes
#include <iosfwd>
#include <string>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints beautiful messages to the console output.                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Logger {

 public:
  // ------------------------------------------------------------------- public static const members
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

  // ------------------------------------------------------------------------- public static members
  static bool printFile;
  static bool printLine;

  static bool enableTrace;   // If any one of these
  static bool enableDebug;   // is set to false,
  static bool enableMessage; // the corresponding
  static bool enableWarning; // messages are discarded.
  static bool enableError;

  // ------------------------------------------------------------------------- public static methods
  static std::ostream& traceImpl(const char* file, int line);
  static std::ostream& debugImpl(const char* file, int line);
  static std::ostream& messageImpl(const char* file, int line);
  static std::ostream& warningImpl(const char* file, int line);
  static std::ostream& errorImpl(const char* file, int line);
};

} // namespace Illusion::Core

// ------------------------------------------------------------------------------------------ macros
// Use these macros in your code like this:
// LOG_MESSAGE << "hello world" << std::endl;
#define ILLUSION_TRACE Illusion::Core::Logger::traceImpl(__FILE__, __LINE__)
#define ILLUSION_DEBUG Illusion::Core::Logger::debugImpl(__FILE__, __LINE__)
#define ILLUSION_MESSAGE Illusion::Core::Logger::messageImpl(__FILE__, __LINE__)
#define ILLUSION_WARNING Illusion::Core::Logger::warningImpl(__FILE__, __LINE__)
#define ILLUSION_ERROR Illusion::Core::Logger::errorImpl(__FILE__, __LINE__)

#endif // ILLUSION_LOGGER_HPP
