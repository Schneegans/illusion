////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_FILE_SYSTEM_HPP
#define ILLUSION_GRAPHICS_FILE_SYSTEM_HPP

#include <string>
#include <sys/stat.h>

namespace Illusion::Core::FileSystem {

time_t getLastWriteTime(std::string const& filename);

} // namespace Illusion::Core::FileSystem

#endif // ILLUSION_GRAPHICS_FILE_SYSTEM_HPP