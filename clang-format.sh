#!/bin/bash

#--------------------------------------------------------------------------------------------------#
#                                                                                                  #
#    _)  |  |            _)                 This software may be modified and distributed          #
#     |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                    #
#    _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                      #
#                                                                                                  #
#   Authors: Simon Schneegans (code@simonschneegans.de)                                            #
#                                                                                                  #
#--------------------------------------------------------------------------------------------------#

# get the location of this script
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

# run clang-format on all source files
find src -iname *.hpp -o -iname *.cpp | xargs clang-format-5.0 -i
find examples -iname *.hpp -o -iname *.cpp | xargs clang-format-5.0 -i
