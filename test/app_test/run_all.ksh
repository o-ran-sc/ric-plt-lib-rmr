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

# run all of the tests, building rmr before the first one if -B is on the command line.

function run_test {
	if [[ -n $capture_file ]]
	then
		if ! ksh $@ >>$capture_file 2>&1
		then
			echo "[FAIL] test failed; see $capture_file"
			(( errors++ ))
		fi
	else
		if ! ksh $@
		then
			(( errors++ ))
		fi
	fi
}

build=""
errors=0
si_flag=""				# eventually we'll default to -S to run SI tests over NNG tests

src_root="../.."
if [[ -d $src_root/.build ]]			# look for build directory in expected places
then									# run scripts will honour this
	export BUILD_PATH=$src_root/.build
else
	if [[ -d $src_root/build ]]
	then
		export BUILD_PATH=$src_root/build
	fi
fi

while [[ $1 == "-"* ]]
do
	case $1 in
		-B)	build="-B";;
		-e)	capture_file=$2; >$capture_file; shift;;
		-i)	installed="-i";;
		-N)	si_flag="";;			# turn on NNG tests (off si)
		-S)	si_flag="-S";;			# turn on si based tests

		*)	echo "'$1' is not a recognised option and is ignored";;
	esac

	shift
done

echo "----- app --------------------"
if which ip >/dev/null 2>&1
then
	run_test run_app_test.ksh $si_flag -v $installed $build
fi

echo "----- multi ------------------"
run_test run_multi_test.ksh $si_flag

echo "----- round robin -----------"
run_test run_rr_test.ksh $si_flag

echo "----- rts -------------------"
run_test run_rts_test.ksh $si_flag -s 5 -d 100

echo "----- extended payload nocopy no clone------"
run_test run_exrts_test.ksh $si_flag -d 10 -n 1000

echo "----- extended payload copy clone------"
run_test run_exrts_test.ksh $si_flag -d 10 -n 1000 -c 11

if (( errors == 0 ))
then
	echo "[PASS] all test pass"
else
	echo "[FAIL] one or more application to application tests failed"
fi

exit $(( !! errors ))
