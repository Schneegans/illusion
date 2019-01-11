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
# which is included in each file. This header has the length of nine lines.
# Still there are some dumb comments which are counted (like those // ------------ things), however
# cloc also does not count inline codes, so this number should be more or less correct.
COMMENT_LINES=$(($COMMENT_LINES - 9 * $NUMBER_OF_FILES))

echo "Lines of code    : $LINES_OF_CODE" 
echo "Lines of comments: $COMMENT_LINES" 

