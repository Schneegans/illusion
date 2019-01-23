#!/bin/bash

# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

# This scripts counts the lines of code and comments in the src/ and example/ directories.
# The copyright-header are substracted. It uses the commandline tool "cloc".

# Get the location of this script
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

# Run cloc - this counts code lines, blank lines and comment lines for the specified languages.
# We are only interested in the summary, therefore the tail -1
SUMMARY="$(cloc ${SCRIPT_DIR}/src ${SCRIPT_DIR}/examples \
               --include-lang="C++,C/C++ Header,GLSL" --md | tail -1)"

# The $SUMMARY is one line of a markdown table and looks like this:
# SUM:|101|3123|2238|10783
# We use the following command to split it into an array.
IFS='|' read -r -a TOKENS <<< "$SUMMARY"

# Store the individual tokens for better readability.
NUMBER_OF_FILES=${TOKENS[1]}
COMMENT_LINES=${TOKENS[3]}
LINES_OF_CODE=${TOKENS[4]}

# To make the estimate of commented lines more accurate, we have to substract the copyright header
# which is included in each file. This header has the length of seven lines.
# Still there are some dumb comments which are counted (like those // ------------ things), however
# cloc also does not count inline comments, so this number should be more or less correct.
COMMENT_LINES=$(($COMMENT_LINES - 7 * $NUMBER_OF_FILES))

awk -v lines=$LINES_OF_CODE 'BEGIN {printf "Lines of code:     %5.1fk\n", lines/1000}'
awk -v lines=$COMMENT_LINES 'BEGIN {printf "Lines of comments: %5.1fk\n", lines/1000}'
