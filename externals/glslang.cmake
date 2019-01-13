# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#    _)  |  |            _)               This code may be used and modified under the terms       #
#     |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                 #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

message("Configuring glslang")

option(SKIP_GLSLANG_INSTALL "Skip installation" On)
add_subdirectory(externals/glslang)

add_library(glslang-default-resource-limits-lib STATIC
            ${CMAKE_CURRENT_SOURCE_DIR}/externals/glslang/StandAlone/ResourceLimits.cpp)
target_include_directories(glslang-default-resource-limits-lib
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/externals/glslang)

message("")
