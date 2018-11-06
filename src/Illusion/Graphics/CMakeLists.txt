#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

# ------------------------------------------------------------------------------------------ compile
file(GLOB FILES_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "*.cpp"
)

add_library(illusion-graphics STATIC ${FILES_SRC})

target_link_libraries(illusion-graphics 
    PUBLIC illusion-core
    PUBLIC illusion-input
    PUBLIC gli
    PUBLIC stb
    PUBLIC glslang
    PUBLIC glslang-default-resource-limits
    PUBLIC spirv-cross
    PUBLIC SPIRV
    PUBLIC Vulkan::Vulkan
)

target_include_directories(illusion-graphics
    PUBLIC ${CMAKE_SOURCE_DIR}/src
)

# ------------------------------------------------------------------------------------------ install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION "include"
  FILES_MATCHING PATTERN "*.hpp"
  PATTERN "internal" EXCLUDE)

install(TARGETS illusion-graphics
  RUNTIME DESTINATION "bin"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib"
)
