# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

add_executable(RunAllTests main.cpp)

target_link_libraries(RunAllTests 
  PRIVATE illusion-core
  PRIVATE illusion-graphics
  PRIVATE illusion-input
  PRIVATE doctest
)

install(TARGETS RunAllTests
  RUNTIME DESTINATION "bin"
)

if (UNIX)
  configure_file("RunAllTests.sh.in" "RunAllTests.sh")

  # install
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/RunAllTests.sh"
    DESTINATION "bin"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
  )
endif()
