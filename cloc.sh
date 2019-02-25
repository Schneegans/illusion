#!/bin/bash

# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

# This scripts counts the lines of code and comments in the test/, src/ and example/ directories.
# The copyright-headers are substracted. It uses the commandline tool "cloc".

# Get the location of this script
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

function countLines() {
  # Run cloc - this counts code lines, blank lines and comment lines for the specified languages.
  # We are only interested in the summary, therefore the tail -1
  SUMMARY="$(cloc $1 --include-lang="C++,C/C++ Header,GLSL" --md | tail -1)"

  # The $SUMMARY is one line of a markdown table and looks like this:
  # SUM:|101|3123|2238|10783
  # We use the following command to split it into an array.
  IFS='|' read -r -a TOKENS <<< "$SUMMARY"

  # Store the individual tokens for better readability.
  NUMBER_OF_FILES=${TOKENS[1]}
  COMMENT_LINES=${TOKENS[3]}
  LINES_OF_CODE=${TOKENS[4]}

  # To make the estimate of commented lines more accurate, we have to substract the copyright header
  # which is included in each file. This header has the length of five lines.
  # All dumb comments like those /////////// or those // ------------ are also substracted. As cloc
  # does not count inline comments, the overall estimate should be rather conservative.
  DUMB_COMMENTS="$(grep -r -E '//////|// -----' $1 | wc -l)"
  COMMENT_LINES=$(($COMMENT_LINES - 5 * $NUMBER_OF_FILES - $DUMB_COMMENTS))

  # Return the two values.
  eval "$2=$LINES_OF_CODE"
  eval "$3=$COMMENT_LINES"
}

# First count the source lines and comment lines in the src/ and examples/ directories.
SOURCE_LOC=""
SOURCE_COM=""
countLines "${SCRIPT_DIR}/src ${SCRIPT_DIR}/examples" SOURCE_LOC SOURCE_COM

# Then in the test/ directory.
TEST_LOC=""
TEST_COM=""
countLines "${SCRIPT_DIR}/test" TEST_LOC TEST_COM

# Print results.
awk -v a=$SOURCE_LOC                'BEGIN {printf "Lines of source code: %5.1fk\n", a/1000}'
awk -v a=$TEST_LOC                  'BEGIN {printf "Lines of test code:   %5.1fk\n", a/1000}'
awk -v a=$SOURCE_COM -v b=$TEST_COM 'BEGIN {printf "Lines of comments:    %5.1fk\n", (a+b)/1000}'
