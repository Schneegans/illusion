#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

option(SKIP_GLSLANG_INSTALL "Skip installation" On)
option(ENABLE_HLSL "Enables HLSL input support" OFF)
option(ENABLE_SPVREMAPPER "Enables building of SPVRemapper" OFF)
option(ENABLE_GLSLANG_BINARIES "Builds glslangValidator and spirv-remap" OFF)

add_subdirectory(externals/glslang)

add_library(glslang-default-resource-limits STATIC
            ${CMAKE_CURRENT_SOURCE_DIR}/externals/glslang/StandAlone/ResourceLimits.cpp)
target_include_directories(glslang-default-resource-limits
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/externals/glslang)
