# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

message("Configuring spirv-tools")

set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/spirv-headers)
set(SKIP_SPIRV_TOOLS_INSTALL ON)
set(SPIRV_SKIP_EXECUTABLES ON)
add_subdirectory(externals/spirv-tools)

message("")
