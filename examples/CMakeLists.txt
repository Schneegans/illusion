#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

# install other resources --------------------------------------------------------------------------
install(DIRECTORY "data"
    DESTINATION "bin"
)

# make each example --------------------------------------------------------------------------------
set(ENABLED_EXAMPLES
  "GltfViewer"
  "ShaderSandbox"
  "TexturedCube"
  "TexturedQuad"
  "Triangle"
)

foreach(EXAMPLE ${ENABLED_EXAMPLES})
  if (UNIX)
    # configure start script
    configure_file("start.sh.in" "${EXAMPLE}.sh")

    # install
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}.sh"
      DESTINATION "bin"
      PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
    )
  endif()
  
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
