////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
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
