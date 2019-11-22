#!/bin/bash

# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

# get the location of this script
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

# run clang-format on all source files
find src examples -iname "*.hpp" -o -iname "*.cpp"|while read file; do
  echo "Formatting ${file}..."
  clang-format -i $file
done
