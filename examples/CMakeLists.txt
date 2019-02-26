# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

# make each example --------------------------------------------------------------------------------
set(ENABLED_EXAMPLES
  "CommandLine"
  "DeferredRendering"
  "GltfViewer"
  "GPUInfo"
  "Headless"
  "ShaderSandbox"
  "SignalSlot"
  "SpecializationConstants"
  "TexturedCube"
  "TexturedQuad"
  "Triangle"
)

foreach(EXAMPLE ${ENABLED_EXAMPLES})
  if (UNIX)
    # configure start script
    configure_file("RunExample.sh.in" "${EXAMPLE}.sh")

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

  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}/data")
    install(DIRECTORY "${EXAMPLE}/data/"
      DESTINATION "bin/data/${EXAMPLE}"
    )
  endif()
endforeach()

if (UNIX)
  configure_file("RunAllExamples.sh.in" "RunAllExamples.sh")

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/RunAllExamples.sh"
    DESTINATION "bin"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
  )
endif()
