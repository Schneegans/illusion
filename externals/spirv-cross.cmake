#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

file(GLOB SPIRV_SRC "${CMAKE_SOURCE_DIR}/externals/spirv-cross/*.cpp")

add_library(spirv-cross STATIC ${SPIRV_SRC})

target_include_directories(spirv-cross
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/externals/spirv-cross
)
