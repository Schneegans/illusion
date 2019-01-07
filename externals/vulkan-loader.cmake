#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

message("Configuring vulkan-loader")

set(VulkanHeaders_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/vulkan-headers/include)
set(VulkanRegistry_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/vulkan-headers/registry)
option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" OFF)
add_subdirectory(externals/vulkan-loader)

message("")
