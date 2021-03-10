#!/usr/bin/env ksh
# vim: ts=4 sw=4 noet :
#==================================================================================
#    Copyright (c) 2019-2020 Nokia
#    Copyright (c) 2018-2020 AT&T Intellectual Property.
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

#	Mnemonic:	run_all.ksh (bash compatable)
#	Abstract:	This script will drive all of the application tests and ensure
#				that the environment is set up as follows:
#
#					Any deb packages which exist in BUILD_PATH are 'installed'
#					into a /tmp directory so that application builds can reference
#					them.
#
#					References are set up to find the NNG library files in the
#					BUILD_PATH directory.
#
#				The BUILD_PATH environment variable should be set, and if not the
#				two directories ../.build and ../build are checked for and used.
#				If the var is not set, and nether of these directories exists,
#				the tests will not be executed.
#
#				At the moment, it assumes a deb based system for tests. 
#
#	Author:		E. Scott Daniels
#	Date:		2019
# -----------------------------------------------------------------------------------

function run_test {
	if [[ -n $capture_file ]]
	then
		if ! $shell $@ >>$capture_file 2>&1
		then
			echo "[FAIL] test failed; see $capture_file"
			(( errors++ ))
		fi
	else
		if ! $shell $@
		then
			(( errors++ ))
		fi
	fi
}

build=""
errors=0

src_root="../.."
if [[ -z $BUILD_PATH ]]						# if not explicitly set, assume one of our standard spots
then
	if [[ -d $src_root/.build ]]			# look for build directory in expected places
	then									# run scripts will honour this
		export BUILD_PATH=$src_root/.build
	else
		if [[ -d $src_root/build ]]
		then
			export BUILD_PATH=$src_root/build
		else
			echo "[ERR]  BUILD_PATH not set and no logical build directory exists to use"
			echo "[INFO] tried: $src_root/build and $src_root/.build"
			exit 1
		fi
	fi
	echo "[INFO] using discovered build directory: $BUILD_PATH"
else
	echo "[INFO] using externally supplied build directory: $BUILD_PATH"
fi

# when dpkg is present, unpack the debs in build so we can reference them. When not
# we assume that the env vars are set properly.
#
if which dpkg >/dev/null 2>&1
then
	goober_dir=/tmp/PID$$.goober	# private playpen for unpacking deb
	rm -fr $goober_dir				# this can fail and we don't care
	if ! mkdir -p $goober_dir		# but we care if this does
	then
		echo "[ERR] run_all: cannot set up working directory for lib/header files: $goober_dir"
		exit 1
	fi

	for d in $BUILD_PATH/*.deb
	do
		echo "[INFO] run_all: unpacking $d"
		dpkg -x $d ${goober_dir}
	done

	find ${goober_dir}

	ginclude=$( find $goober_dir -name include | head -1 )
	glib=$( find $goober_dir -name lib | head -1 )
	export C_INCLUDE_PATH=$BUILD_PATH/include:${ginclude}:$C_INCLUDE_PATH
	export LIBRARY_PATH=$BUILD_PATH:$BUILD_PATH/.xbuild/lib:${glib}:$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH=$LIBRARY_PATH
fi


if whence ksh >/dev/null 2>&1
then
	shell=ksh
else
	shell=bash
fi

verbose=0
purge=1
while [[ $1 == "-"* ]]
do
	case $1 in
		-B)	build="-b";;			# build RMR without pulling
		-e)	capture_file=$2; >$capture_file; shift;;
		-i)	installed="-i";;
		-N)	;;						# ignored for backwards compatability; nng no longer supported
		-P)	build="-B";;			# build RMR with a pull first
		-p)	purge=0;;				# don't purge binaries to ensure rebuild happens
		-S)	;;						# ignored; nng tests are not supported so all binaries are now si95
		-s) shell=$2; shift;;
		-v)	verbose=1;;

		-\?) echo "usage: $0 [-B|-P] [-e err_file] [-i] [-p] [-s shell] [-v]";;

		*)	echo "'$1' is not a recognised option and is ignored";;
	esac

	shift
done

if (( verbose ))
then
	env | grep PATH
	if [[ -n $goober_dir ]]
	then
		find $goober_dir
	fi
fi

export SHELL=$shell

if (( purge ))
then
	rm -f sender receiver 
fi

echo "----- app --------------------"
if which ip >/dev/null 2>&1					# ip command rquired for the app test; skip if not found
then
	run_test run_app_test.sh -v $installed $build
	build=""
fi

echo "----- multi ------------------"
run_test run_multi_test.sh  $build

echo "----- round robin -----------"
run_test run_rr_test.sh 

echo "----- rts -------------------"
run_test run_rts_test.sh  -s 5 -d 100

echo "----- extended payload nocopy no clone------"
run_test run_exrts_test.sh  -d 10 -n 1000

echo "----- extended payload copy clone------"
run_test run_exrts_test.sh  -d 10 -n 1000 -c 11

if (( errors == 0 ))
then
	echo "[PASS] all test pass"
else
	echo "[FAIL] one or more application to application tests failed"
fi


rm -fr goober_dir
exit $(( !! errors ))
