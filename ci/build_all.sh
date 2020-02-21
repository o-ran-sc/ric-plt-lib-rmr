#!/usr/bin/env bash
# :vim ts=4 sw=4 noet:
#==================================================================================
#    Copyright (c) 2020 Nokia
#    Copyright (c) 2020 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#==================================================================================
#

#	Mnemonic:	build_all.sh
#	Abstract:	To setup CMake and build multiple packages with varying content
#				(dev and runtime) it may be easier to run a single script.
#				This is that script which will:
#					1) make the .build directory
#					2) run cmake to configure the dev package
#					3) build the dev package
#					4) run cmake to conigure the runtime package
#					5) build the runtime package
#					6) run unit tests
#
#				Assumptions:
#					We assume that this scirpt is executed at the 'root' of the
#					RMr repo (i.e. the directory which has a subdirectory ci).
#					e.g.  bash ci/build_all.sh
#
#				Coverage Files
#				As a part of unit testing, coverage files are left in /tmp/rmr_gcov
#				which can be overridden by setting the environment variable
#				UT_COVERAGE_DIR.
#
#	Returns:	The exit code will be non-zero on failure, and 0 if all builds and
#				the tests pass.
#
#	Date:		28 February 2010
# --------------------------------------------------------------------------------

set -e 	# lazy error checking
mkdir -p .build
cd .build
cmake .. -DDEV_PKG=1 -DBUILD_DOC=1
make package install
cmake .. -DDEV_PKG=0
make package install
make test ARGS="-V"

exit $?

