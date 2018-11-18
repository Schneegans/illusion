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
  "ModelViewer"
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

  # find sources
  file(GLOB_RECURSE EXAMPLE_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${EXAMPLE}/*.cpp"
  )

  # compile and link
  add_executable(${EXAMPLE} ${EXAMPLE_SRC})
  # add_dependencies(${EXAMPLE} ExtractReflection)

  # target_include_directories(${EXAMPLE}
  #   PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/resources"
  #   PRIVATE "${CMAKE_SOURCE_DIR}/src"
  #   PUBLIC ${INCLUDE_DIRS}
  # )

  target_link_libraries(${EXAMPLE} 
    PRIVATE illusion-graphics
  )

  install(TARGETS ${EXAMPLE}
    RUNTIME DESTINATION "bin"
  )
endforeach()
