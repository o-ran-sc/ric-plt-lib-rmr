#!/bin/bash
#/usr/bin/env ksh
# vim: ts=4 sw=4 noet :
#==================================================================================
#    Copyright (c) 2019-2021 Nokia
#    Copyright (c) 2018-2021 AT&T Intellectual Property.
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
#	Mnemonic:	run_app_test.ksh
#	Abstract:	This is a simple script to set up and run the basic send/receive
#				processes for some library validation on top of nng.
#				It should be possible to clone the repo, switch to this directory
#				and execute  'ksh run -B'  which will build RMr, make the sender and
#				recevier then  run the basic test.
#
#				Example command line:
#					ksh ./run_app_test.ksh		# default 20 messages at 2 msg/sec
#					ksh ./run_app_test.ksh -d 100 -n 10000 # send 10k messages with 100ms delay between
#
#	Date:		22 April 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------


# The sender and receiver are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily.
#
function run_sender {

#	first we send an invalid message with a negative message length 
#	as per RIC-989 / CVE-2023-40998 processes used to  crash for such 
#	invalid size data. So this test case would fail with a crash
#	if for some reason the bug comes back.
#
  bash ./create_invalid_request.sh
  cat /tmp/invalid_data.bin |netcat -w 2 127.0.0.1 4560
	
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch

  
#      now let's send still one valid message over another TCP 
#      connection
  bash ./create_valid_request.sh
  cat /tmp/valid_data.bin|netcat -w 2 127.0.0.1 4560

}

function run_rcvr {
	export RMR_LOG_VLEVEL=5
	if (( mt_receiver ))
	then
		echo "<TEST> testing with mt-receiver" >&2
		./mt_receiver${si} $nmsg
	else
		./receiver${si} $nmsg
	fi
	echo $? >/tmp/PID$$.rrc
}

# snarf the first v4 IP (not the loopback) that belongs to this box/container/guest
function snarf_ip {
	ip addr| sed -e '/inet /b c; d' -e ':c' -e '/127.0.0.1/d; s!/.*!!; s!^.* !!; q'
}

#	Drop a contrived route table in. This table should add a reference to our 
#	local IP to an entry to ensure that the route table collector code in RMr
#	is removing 'hairpin' loops. If RMr isn't removing the references to our
#	hostname and IP address when it builds the endpoint lists, the sender will
#	send messages to itself some of the time, causing the receiver to come up 
#	short when comparing messages received with expected count and thus failing.
#
function set_rt {
	typeset port=4560			# port the receiver listens on by default

	cat <<endKat >app_test.rt
		newrt | start
			mse | 0 |  0 | 127.0.0.1:$port,$my_ip:43086
			mse | 1 | 10 | 127.0.0.1:$port,${my_host//.*/}:43086
			mse | 2 | 20 | 127.0.0.1:$port
			rte | 3 | 127.0.0.1:$port
			mse | 3 | 100 | 127.0.0.1:$port	# special test to ensure that this does not affect previous entry
			rte | 4 | 127.0.0.1:$port
			rte | 5 | 127.0.0.1:$port
			rte | 6 | 127.0.0.1:$port
			rte | 7 | 127.0.0.1:$port
			rte | 8 | 127.0.0.1:$port
			rte | 9 | 127.0.0.1:$port
			rte | 10 | 127.0.0.1:$port
			rte | 11 | 127.0.0.1:$port
			rte | 12 | 127.0.0.1:$port
			rte | 13 | 127.0.0.1:$port
		newrt | end

head -3 app_test.rt

endKat

}

# ---------------------------------------------------------

nmsg=1						# total number of messages to be exchanged (-n value changes)
							# need two sent to each receiver to ensure hairpin entries were removed (will fail if they were not)
delay=100000				# microsec sleep between msg 1,000,000 == 1s
wait=1
rebuild=0
nopull=""					# -b sets so that build does not pull
verbose=0
use_installed=0
my_ip=$(snarf_ip)			# get an ip to insert into the route table
keep=0
mt_receiver=0				# -m sets in order to test with multi-threaded receive stuff
force_make=0

echo --------------------
echo running test for negative message size
echo --------------------


while [[ $1 == -* ]]
do
	case $1 in
		-B)	rebuild=1;;						# build with pull first
		-b)	rebuild=1; nopull="nopull";;	# buld without pull
		-d)	delay=$2; shift;;
		-k) keep=1;;
		-i) use_installed=1;;
		-M)	force_make=1;;
		-m)	mt_receiver=1;;
		-n)	nmsg=$2; shift;;
		-N)	;;							# ignored for back compat; NNG not supported; all binaries are si95
		-S)	;;							# ignored for back compat; all binaries are si95 and no _si suffix is used
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-d micor-sec-delay] [-i] [-k] [-M] [-m] [-n num-msgs] [-S]"
			echo "  -B forces an RMR rebuild which will use .build"
			echo "  -i causes the installd libraries (/usr/local) to be referenced; -B is ignored if supplied"
			echo "  -k keeps the route table"
			echo "  -M force make on test applications"
			echo "  -m test with mt-receive mode"
			echo "  -S build/test SI95 based binaries"
			echo ""
			echo "total number of messages must > 20 to correctly test hairpin loop removal"
			exit 1
			;;
	esac

	shift
done

if [[ ! -f app_test.rt ]]		# we need the real host name in the local.rt; build one from mask if not there
then
	my_host=$(hostname)
	set_rt
	if (( verbose ))
	then
		cat app_test.rt
	fi
fi

if (( verbose ))
then
	echo "4" >.verbose
	export RMR_VCTL_FILE=".verbose"
fi


if (( use_installed ))			# point at installed library rather than testing the build
then
	export LD_LIBRARY_PATH=/usr/local/lib
	export LIBRARY_PATH=$LD_LIBRARY_PATH
else
	if (( rebuild ))
	then
		set -e
		$SHELL ./rebuild.ksh $nopull | read build_path
		set +e
	else
		build_path=${BUILD_PATH:-"../../.build"}	# we prefer .build at the root level, but allow user option
	
		if [[ ! -d $build_path ]]
		then
			echo "[FAIL] cannot find build in: $build_path"
			echo "[FAIL] either create, and then build RMr, or set BUILD_PATH as an evironment var before running this"
			exit 1
		fi
	fi

	if [[ -z $LD_LIBRARY_PATH ]]		# the cmake test environment will set this; we must use if set
	then
		if [[ -d $build_path/lib64 ]]
		then
			export LD_LIBRARY_PATH=$build_path:$build_path/lib64
		else
			export LD_LIBRARY_PATH=$build_path:$build_path/lib
		fi
	fi
	export LIBRARY_PATH=$LD_LIBRARY_PATH
fi

export RMR_SEED_RT=${RMR_SEED_RT:-./app_test.rt}		# allow easy testing with different rt

if (( force_make )) || [[ ! -f ./sender${si} || ! -f ./receiver${si} ]]
then
	if ! make -B sender${si} receiver${si} >/tmp/PID$$.clog 2>&1
	then
		echo "[FAIL] cannot find sender${si} and/or receiver${si} binary, and cannot make them.... humm?"
		echo "[INFO] ------------- PATH settings -----------------"
		env | grep PATH
		echo "[INFO] ------------- compiler output (head) -----------------"
		head -50 /tmp/PID$$.clog
		rm -fr /tmp/PID$$.*
		exit 1
	fi
fi

#exit
run_rcvr &
sleep 2				# if sender starts faster than rcvr we can drop msgs, so pause a bit
netstat -nptl|grep receiver 
netstat -nptl|grep sender 

run_sender &

wait
head -1 /tmp/PID$$.rrc | read rrc
head -1 /tmp/PID$$.src | read src

if (( !! (src + rrc) ))
then
	echo "[FAIL] sender rc=$src  receiver rc=$rrc"
else
	echo "[PASS] sender rc=$src  receiver rc=$rrc"
fi

if (( ! keep )) 
then
	rm app_test.rt
fi

rm /tmp/PID$$.*
#rm -f .verbose

exit $(( !! (src + rrc) ))

