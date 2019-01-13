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

add_library(illusion-input STATIC ${FILES_SRC})

target_link_libraries(illusion-input 
    PUBLIC illusion-core
    PUBLIC glfw
)

target_include_directories(illusion-input
    PUBLIC ${CMAKE_SOURCE_DIR}/src
)

# install ------------------------------------------------------------------------------------------
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION "include/Illusion"
  FILES_MATCHING PATTERN "*.hpp"
  PATTERN "internal" EXCLUDE)

install(TARGETS illusion-input
  RUNTIME DESTINATION "bin"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib"
)
