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

# This is a wrappter to the unit test script which is needed because CMake test
# is unable to set environment variables before executing the test command.
# this script assumes that env variables are passed on the command line in x=y
# format. We will set all that are given; the only exception to this is that
# we will NOT override the BUILD_PATH variable which allows it to be set 
# from the script that invokes 'make test'

# We assume that the CMBUILD variable has a good build directory (e.g. .build)
# but the user can be sure by hard setting BUILD_PATH which will NOT be
# changed.  For example:
#	cd rmr/.bld
#	BUILD_PATH=$PWD make test ARGS="-V"

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

bash ./unit_test.ksh -q
