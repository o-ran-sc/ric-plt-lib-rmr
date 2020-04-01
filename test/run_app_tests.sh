#!/usr/env bash
# vim: ts=4 sw=4 noet :
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

# this is a simple wrapper until we figure out why CMake seems unable to start
# the run_all script in the app test directory. Even after that it will likely
# be needed because it seems impossible to set environment vars from CMake for
# the test script which makes testing alternate install locations impossible.
# So, we must jump some hoops to allow for that...

# Hoop:
# It seems that we cannot supply env vars on the CMake generated command :(
# To deal with this, all leading positional parms of the form foo=bar are
# looked at and if we like it we'll export it before starting the test.

# It is also impossible for us to know what the build directory the user
# created for their build and test. We will auto discover if they used
# .build or build in the parent to this directory (in that order). If a
# different directory is desired, then the build directory must be supplied
# as an environment variable to the make:
#	cd rmr/.bld
#	BUILD_PATH=$PWD make test ARGS=-v

while [[ $1 == *"="* ]]
do
	case ${1%%=*} in
		CMBUILD)						# should be cmake build dir
			if [[ -z $BUILD_PATH ]]		# still allow user to override
			then
				export BUILD_PATH=${1##*=}
			fi
			;;

		LD_LIBRARY_PATH)
			export LD_LIBRARY_PATH=${1##*=}
			;;

		C_INCLUDE_PATH)
			export C_INCLUDE_PATH=${1##*=}
			;;

		LIBRARY_PATH)
			export LIBRARY_PATH=${1##*=}
			;;
	esac

	shift
done


set -e
cd app_test
bash ./run_all.sh  -S	# build CI likely doesn't have ksh; Run SI tests
