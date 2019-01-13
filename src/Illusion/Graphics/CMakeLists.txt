# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#    _)  |  |            _)               This code may be used and modified under the terms       #
#     |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                 #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

# compile ------------------------------------------------------------------------------------------
file(GLOB FILES_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "*.cpp"
)

add_library(illusion-graphics STATIC ${FILES_SRC})

target_link_libraries(illusion-graphics 
    PUBLIC illusion-core
    PUBLIC illusion-input
    PUBLIC gli
    PUBLIC stb
    PUBLIC tinygltf
    PUBLIC glslang
    PUBLIC glslang-default-resource-limits-lib
    PUBLIC spirv-cross
    PUBLIC SPIRV
    PUBLIC vulkan
    PUBLIC vulkan-headers
)

target_include_directories(illusion-graphics
    PUBLIC ${CMAKE_SOURCE_DIR}/src
)

# install ------------------------------------------------------------------------------------------
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION "include/Illusion"
  FILES_MATCHING PATTERN "*.hpp"
  PATTERN "internal" EXCLUDE)

install(TARGETS illusion-graphics
  RUNTIME DESTINATION "bin"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib"
)
