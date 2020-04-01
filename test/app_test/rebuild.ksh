#!/usr/bin/env ksh
# vim: ts=4 sw=4 noet :
#==================================================================================
#    Copyright (c) 2019 Nokia
#    Copyright (c) 2018-2019 AT&T Intellectual Property.
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

# ---------------------------------------------------------------------------------
#	Mnemonic:	rebuild.ksh
#	Abstract:	This is a simple script that will cause RMR to be rebuilt. It
#				may be invoked by any of the run_* scripts in this directory.
#
#				To facilitate a single step build and test process for CI this
#				script assumes that the following has beeen executed in the BUILD_PATH
#				directory:
#					cmake .. -DDEV_PKG=0 -DCMAKE_INSTALL_PREFIX=<path>
#
#				Given that this script will compile, install and package the
#				RMR code (this builds the runtime packages).  It will then reconfigure
#				CMake to generate the dev package and generate the dev package.
#
#				NOTE:
#				The build path is echod onto stdout so that the caller is able
#				to reference build items for compile/linking. All other communication
#				should be directed to stderr.
#
#	Date:		24 April 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------


parent=${PWD%/*}					# allow us to step up gracefully
gparent=${parent%/*}
build_path=${BUILD_PATH:-${gparent}/.build} 	# where we should build; .build by default

echo "$(date) build starts" >&2
(
	set -e
	mkdir -p $build_path
	cd $gparent
	if [[ $1 != "nopull" ]]				# pull by default, but for local dev testing this needs to be avoided
	then
		git pull						# get the up to date code so if run from an old image it's a good test
	fi
	cd $build_path
	make install 								# install for tests and build package in some local, non-root place for CI
	cmake .. -DPACK_EXTERNALS=1 -DDEV_PKG=1		# must force nng to be finable
	make install 								# must install rmr headers so need both; again non-root location for CI

	cmake .. -DDEV_PKG=0	-DCMAKE_PACKAGE_PREFIX=/usr/local		# must build packages WITHOUT the CI prefix
	make package
	cmake .. -DDEV_PKG=1
	make package
) >/tmp/PID$$.log
if (( $? != 0 ))
then
	cat /tmp/PID$$>log
	echo "$(date) build failed" >&2
	exit 1
fi

echo "$(date) build completed" >&2
echo "$build_path"

