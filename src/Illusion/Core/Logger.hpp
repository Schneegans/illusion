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

#include <iosfwd>
#include <string>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Prints beautiful messages to the console output. You can use the macros at the bottom of this  //
// file like this:                                                                                //
// LOG_MESSAGE << "hello world" << std::endl;                                                     //
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

  // Modify those to enable or disable printing of file names or line numbers.
  static bool printFile;
  static bool printLine;

  // If any one of these is set to false, the corresponding messages are discarded.
  static bool enableTrace;
  static bool enableDebug;
  static bool enableMessage;
  static bool enableWarning;
  static bool enableError;

  // These are used by the marcos below. You can also use them directly, however using the macros
  // might be a better idea.
  static std::ostream& traceImpl(const char* file, int line);
  static std::ostream& debugImpl(const char* file, int line);
  static std::ostream& messageImpl(const char* file, int line);
  static std::ostream& warningImpl(const char* file, int line);
  static std::ostream& errorImpl(const char* file, int line);
};

} // namespace Illusion::Core

// Use these macros in your code like this:
// LOG_MESSAGE << "hello world" << std::endl;
#define ILLUSION_TRACE Illusion::Core::Logger::traceImpl(__FILE__, __LINE__)
#define ILLUSION_DEBUG Illusion::Core::Logger::debugImpl(__FILE__, __LINE__)
#define ILLUSION_MESSAGE Illusion::Core::Logger::messageImpl(__FILE__, __LINE__)
#define ILLUSION_WARNING Illusion::Core::Logger::warningImpl(__FILE__, __LINE__)
#define ILLUSION_ERROR Illusion::Core::Logger::errorImpl(__FILE__, __LINE__)

#endif // ILLUSION_LOGGER_HPP
