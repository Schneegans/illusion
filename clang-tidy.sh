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

FLAGS="-std=c++17 "
FLAGS+="-I${SCRIPT_DIR}/src "
FLAGS+="-I${SCRIPT_DIR}/externals/glm "
FLAGS+="-I${SCRIPT_DIR}/externals/gli "
FLAGS+="-I${SCRIPT_DIR}/externals/stb "
FLAGS+="-I${SCRIPT_DIR}/externals/tinygltf "
FLAGS+="-I${SCRIPT_DIR}/externals/glslang "
FLAGS+="-I${SCRIPT_DIR}/externals/spirv-cross "
FLAGS+="-I${SCRIPT_DIR}/externals/vulkan-headers/include "
FLAGS+="-I${SCRIPT_DIR}/externals/entt/src"

CHECKS="modernize-*,"
CHECKS+="bugprone-*,"
CHECKS+="readability-*,"
CHECKS+="performance-*,"
CHECKS+="hicpp-*,"
CHECKS+="misc-*,"
CHECKS+="cppcoreguidelines-*,"

# run clang-tidy on all source files
find src examples -iname "*.cpp"|while read file; do
  echo "Tidying ${file}..."
  clang-tidy-7 -checks=$CHECKS -quiet $file -- $FLAGS
done
