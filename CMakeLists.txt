#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.8)
project(illusion VERSION 0.0.1 LANGUAGES CXX)

# global settings ----------------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# build dependencies -------------------------------------------------------------------------------

message("")
message("Configuring submodules")
message("")

include(externals/glslang.cmake)
include(externals/vulkan-loader.cmake)
include(externals/spirv-tools.cmake)
include(externals/spirv-cross.cmake)
include(externals/vulkan-headers.cmake)
include(externals/vulkan-validation-layers.cmake)
include(externals/glfw.cmake)
include(externals/glm.cmake)
include(externals/gli.cmake)
include(externals/stb.cmake)
include(externals/tinygltf.cmake)

# build the libraries ------------------------------------------------------------------------------
add_subdirectory(src/Illusion)

# build the examples -------------------------------------------------------------------------------
option(ILLUSION_COMPILE_SAMPLES "Compile examples" ON)

if(ILLUSION_COMPILE_SAMPLES)
    add_subdirectory(examples)
endif()
