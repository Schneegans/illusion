#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

# -------------------------------------------------------------------------- install other resources
install(DIRECTORY "data"
    DESTINATION "bin"
)

# -------------------------------------------------------------------------------- make each example
set(ENABLED_EXAMPLES
  # "GltfLoader"
  "TexturedCube"
  "TexturedQuad"
  "Triangle"
)

foreach(EXAMPLE ${ENABLED_EXAMPLES})
  file(GLOB_RECURSE EXAMPLE_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${EXAMPLE}/*.cpp"
  )

  add_executable(${EXAMPLE} ${EXAMPLE_SRC})

  target_link_libraries(${EXAMPLE} 
    PRIVATE illusion-graphics
  )

  install(TARGETS ${EXAMPLE}
    RUNTIME DESTINATION "bin"
  )
endforeach()
