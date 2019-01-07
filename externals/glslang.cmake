#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

message("Configuring glslang")

option(SKIP_GLSLANG_INSTALL "Skip installation" On)
add_subdirectory(externals/glslang)

add_library(glslang-default-resource-limits-lib STATIC
            ${CMAKE_CURRENT_SOURCE_DIR}/externals/glslang/StandAlone/ResourceLimits.cpp)
target_include_directories(glslang-default-resource-limits-lib
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/externals/glslang)

message("")
