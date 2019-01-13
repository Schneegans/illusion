# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

file(GLOB FILES_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "externals/spirv-cross/*.cpp"
)

add_library(spirv-cross STATIC ${FILES_SRC})
target_include_directories(spirv-cross
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/externals/spirv-cross
)
