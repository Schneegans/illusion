# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#    _)  |  |            _)               This code may be used and modified under the terms       #
#     |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                 #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

message("Configuring vulkan-validation-layers")

set(Vulkan-Headers_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/vulkan-headers)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_CURRENT_BINARY_DIR}/externals/vulkan-headers/install)
option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" OFF)
add_subdirectory(externals/vulkan-validation-layers)

message("")
