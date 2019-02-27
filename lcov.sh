#!/bin/bash

# ------------------------------------------------------------------------------------------------ #
#                                                                                                  #
#     _)  |  |            _)                This code may be used and modified under the terms     #
#      |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details.  #
#     _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans               #
#                                                                                                  #
# ------------------------------------------------------------------------------------------------ #

# This script uses lcov to capture the source coverage of the test executed with RunAllTests.sh. If
# you pass any argument to the script (say ./lcov.sh foo) then it will create also an html report
# and open this report in your web browser.

# Get the location of this script.
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
cd $SCRIPT_DIR

# Create initial zero-coverage report.
lcov -q --zerocounters --directory .
lcov -q --capture --no-external --initial --directory . --output-file build/zero_coverage.info

# Run all tests
build/install/bin/RunAllTests.sh

# Capture the coverage of the test.
lcov -q --capture --no-external --directory . --output-file build/test_coverage.info
lcov -q -a build/zero_coverage.info -a build/test_coverage.info --o build/coverage.info

# Remove any coverage from externals, examples and test directories.
lcov -q --remove build/coverage.info \*externals/\* --output-file build/coverage.info
lcov -q --remove build/coverage.info \*examples/\* --output-file build/coverage.info
lcov -q --remove build/coverage.info \*test/\* --output-file build/coverage.info

# Generate html report and open it in a web browser when an argument was passed to the script.
if [ $# != 0 ]
  then
    genhtml build/coverage.info --output-directory build/coverage
    xdg-open build/coverage/index.html
fi
