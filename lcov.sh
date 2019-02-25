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
cd $SCRIPT_DIR

lcov -q --zerocounters --directory .
lcov -q --capture --no-external --initial --directory . --output-file build/zero_coverage.info

build/install/bin/RunAllTests.sh

lcov -q --capture --no-external --directory . --output-file build/test_coverage.info
lcov -q -a build/zero_coverage.info -a build/test_coverage.info --o build/coverage.info

lcov -q --remove build/coverage.info \*externals/\* --output-file build/coverage.info
lcov -q --remove build/coverage.info \*examples/\* --output-file build/coverage.info
lcov -q --remove build/coverage.info \*test/\* --output-file build/coverage.info
