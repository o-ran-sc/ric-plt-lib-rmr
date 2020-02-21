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

# ---------------------------------------------------------------------------------
#	Mnemonic:	run_call_test.ksh
#	Abstract:	This is a simple script to set up and run the basic send/receive
#				processes for some library validation on top of nng.
#				It should be possible to clone the repo, switch to this directory
#				and execute  'ksh run -B'  which will build RMr, make the sender and
#				recevier then  run the basic test.
#
#				The call test drives three RMr based processes:
#					caller which invokes rmr_mt_call() to send n messages
#
#					receiver which executes rmr_rts_msg() on each received
#						message with type == 5.
#	
#					sender which sends n "pings" to the caller process
#
#				The script assumes that all three proessess are running on the same
#				host/container such that setting up the route table with localhost:port
#				will work.
#
#				The number of message, and the delay between each message may be given
#				on the command line. The number of threads may also be given. The
#				number of messages defines the number _each_ caller will sent (5000 
#				messages and three threads == 15,000 total messages.
#
#				Example command line:
#					ksh ./run_call_test.ksh		# default 10 messages at 1 msg/sec 3 threads
#					ksh ./run_call_test.ksh -n 5000  -t 6 -d 400	# 5k messages, 6 threads, delay 400 mus
#
#	Date:		20 May 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------


# The sender and receiver are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily.
#
function run_sender {
	./caller${si} $nmsg $delay $nthreads
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch
}

# start receiver listening for nmsgs from each thread
function run_rcvr {
	./mt_receiver${si} $(( nmsg * nthreads ))		# we'll test with the RMr multi threaded receive function
	echo $? >/tmp/PID$$.rrc
}

# This will send 100 messages to the caller process. This is a test to verify that
# threaded calling is not affected by normal messages and that normal messages can
# be received concurrently.
#
function run_pinger {
    RMR_RTG_SVC=9999 ./sender${si} 100 100 4 3333 >/dev/NULL 2>&1  # send pings
}


# Generate a route table that is tailored to our needs of sender sending messages to
# the caller, and caller sending mtype == 5 to the receiver.
#
function mk_rt {

cat <<endKat >caller.rt
# this is a specialised rt for caller testing. mtype 5 go to the
# receiver, and all others go to the caller.

newrt | start
mse | 0 |  0 | $localhost:43086
mse | 1 | 10 | $localhost:43086
mse | 2 | 20 | $localhost:43086
rte | 3 | $localhost:43086
mse | 3 | 100 | $localhost:43086	# special test to ensure that this does not affect previous entry
rte | 4 | $localhost:43086
rte | 5 | $localhost:4560
rte | 6 | $localhost:43086
rte | 7 | $localhost:43086
rte | 8 | $localhost:43086
rte | 9 | $localhost:43086
rte | 10 | $localhost:43086
rte | 11 | $localhost:43086
rte | 12 | $localhost:43086
rte | 13 | $localhost:43086

newrt | end | 16
endKat
}

# ---------------------------------------------------------

nmsg=10						# total number of messages to be exchanged (-n value changes)
delay=1000000				# microsec sleep between msg 1,000,000 == 1s
wait=1
rebuild=0
verbose=0
nthreads=3
dev_base=1					# -D turns off to allow this to run on installed libs
force_make=0
si=""
localhost="localhost"



while [[ $1 == -* ]]
do
	case $1 in
		-B)	rebuild=1;;
		-d)	delay=$2; shift;;
		-D)	dev_base=0;;
		-n)	nmsg=$2; shift;;
		-M)	force_make=1;;
		-N) si="";;						# enable NNG testing (off si)
		-S) si="_si"					# enable SI95 testing
			localhost="127.0.0.1"
			;;
		-t)	nthreads=$2; shift;;
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-d micro-sec-delay] [-M] [-n num-msgs] [-S] [-t num-threads]"
			echo "  -B forces an RMR rebuild which will use .build"
			echo "  -M force test applications to rebuild"
			echo "  -S build/test SI95 based binaries"
			exit 1
			;;
	esac

	shift
done

if (( verbose ))
then
	echo "2" >.verbose
	export RMR_VCTL_FILE=".verbose"
fi

if (( rebuild ))
then
	build_path=../../.build
	set -e
	$SHELL ./rebuild.ksh
	set +e
else
	build_path=${BUILD_PATH:-"../../.build"}	# we prefer .build at the root level, but allow user option

	if [[ ! -d $build_path ]]
	then
		echo "cannot find build in: $build_path"
		echo "either create, and then build RMr, or set BUILD_PATH as an evironment var before running this"
		exit 1
	fi
fi

if (( dev_base ))							# assume we are testing against what we've built, not what is installed
then
	if [[ -d $build_path/lib64 ]]
	then
		export LD_LIBRARY_PATH=$build_path:$build_path/lib64:$LD_LIBRARY_PATH
	else
		export LD_LIBRARY_PATH=$build_path:$build_path/lib:$LD_LIBRARY_PATH
	fi
	export LIBRARY_PATH=$LD_LIBRARY_PATH
else										# -D option gets us here to test an installed library
	export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
	export LIBRARY_PATH=$LD_LIBRARY_PATH
fi

export RMR_SEED_RT=${RMR_SEED_RT:-./caller.rt}		# allow easy testing with different rt

if [[ ! -f $RMR_SEED_RT ]]							# special caller rt for three process setup
then
	mk_rt
fi

if (( rebuild || force_make )) || [[ ! -f ./sender${si} || ! -f ./caller${si} || ! -f ./mt_receiver${si} ]]
then
	if ! make -B sender${si} caller${si} mt_receiver${si} >/tmp/PID$$.log 2>&1
	then
		echo "[FAIL] cannot find caller{$si} and/or mt_receiver${si} binary, and cannot make them.... humm?"
		cat /tmp/PID$$.log
		rm -f /tmp/PID$$.*
		exit 1
	fi
fi

run_rcvr &
sleep 2				# if caller starts faster than rcvr we can drop, so pause a bit
run_sender &
run_pinger &

wait
head -1 /tmp/PID$$.rrc | read rrc		# get pass/fail state from each
head -1 /tmp/PID$$.src | read src

if (( !! (src + rrc) ))
then
	echo "[FAIL] sender rc=$src  receiver rc=$rrc"
else
	echo "[PASS] sender rc=$src  receiver rc=$rrc"
fi

rm /tmp/PID$$.*
rm -f .verbose

exit $(( !! (src + rrc) ))

