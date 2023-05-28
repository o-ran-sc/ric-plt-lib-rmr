#!/usr/bin/env ksh
# vim: ts=4 sw=4 noet :
#==================================================================================
#    Copyright (c) 2019-2021 Nokia
#    Copyright (c) 2018-2021 AT&T Intellectual Property
#    Copyright (c) 2023 Alexandre Huff
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
#	Mnemonic:	run_ipv6_test.sh
#	Abstract:	This is a simple script to set up and run the basic send/receive
#				processes using IPv6 for some library validation on top of si95.
#				This particular test starts a sender and a receiver application.
#				All messages go to the receiver and an ack is sent back to the
#				sending process.
#
#				The number of message, and the delay between each message may be given
#				on the command line.
#
#				Example command line:
#					bash ./run_ipv6_test.sh		# default 10 messages at 1 msg/sec
#					bash ./run_ipv6_test.sh -n 10 -d 100000 # 10 messages, delay 100 ms
#
#	Date:		28 May 2023
#	Author:		Alexandre Huff
# ---------------------------------------------------------------------------------


# The sender and receiver are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily.
#
function run_sender {
	./sender${si} $nmsg $delay
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch
}

# start receiver listening for nmsgs from each thread
function run_rcvr {
	./receiver${si} $nmsg
	echo $? >/tmp/PID$$.rrc
}

# Generate a route table that is tailored to our needs.
#
function mk_rt {

cat <<endKat >ipv6.rt
# This is a route table to test IPv6 support with sender and receiver

newrt | start
mse | 0 |  0 | $localhost:4560
mse | 1 | 10 | $localhost:4560
mse | 2 | 20 | $localhost:4560
rte | 3 | $localhost:4560
rte | 4 | $localhost:4560
rte | 5 | $localhost:4560
rte | 6 | $localhost:4560
rte | 7 | $localhost:4560
rte | 8 | $localhost:4560
rte | 9 | $localhost:4560
newrt | end
endKat
}

function ensure_ipv6 {
	v6=$(ip -6 address show lo)
	if [[ ! -z $v6 ]]
	then
		v4=$(ip -4 address show lo)
		if [[ ! -z $v4 ]]
		then
			export RMR_BIND_IF=$localhost	# force RMR to bind to an IPv6 address (defaults to IPv4 on dual stack)
			echo "[INFO] forcing RMR binding to $localhost"
		else
			echo "[INFO] using IPv6-only stack"	# RMR binds to any interface address, no need to specify RMR_BIND_IF
		fi
	else
		echo "[WARN] skipping IPv6 test, unable to detect IPv6 stack"
		exit 0	# exit success in favor to allow overall tests to pass if all other tests have passed
	fi
}

# ---------------------------------------------------------

nmsg=10						# total number of messages to be exchanged (-n value changes)
delay=1000000				# microsec sleep between msg 1,000,000 == 1s
wait=1
rebuild=0
verbose=0
dev_base=1					# -D turns off to allow this to run on installed libs
force_make=0
si=""
localhost="[::1]"



while [[ $1 == -* ]]
do
	case $1 in
		-B)	rebuild=1;;
		-d)	delay=$2; shift;;
		-D)	dev_base=0;;
		-n)	nmsg=$2; shift;;
		-M)	force_make=1;;
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-M] [-d micro-sec-delay] [-n num-msgs]"
			echo "  -B forces an RMR rebuild"
			echo "  -M force test applications to rebuild"
			exit 1
			;;
	esac

	shift
done

ensure_ipv6

if (( verbose ))
then
	echo "2" >.verbose
	export RMR_VCTL_FILE=".verbose"
fi

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

if (( rebuild ))
then
	set -e
	$SHELL ./rebuild.sh
	set +e
fi

if [[ -z $LD_LIBRARY_PATH ]]					# cmake test will set and it must be honoured
then
	if (( dev_base ))							# assume we are testing against what we've built, not what is installed
	then
		if [[ -d $BUILD_PATH/lib64 ]]
		then
			export LD_LIBRARY_PATH=$BUILD_PATH:$BUILD_PATH/lib64:$LD_LIBRARY_PATH
		else
			export LD_LIBRARY_PATH=$BUILD_PATH:$BUILD_PATH/lib:$LD_LIBRARY_PATH
		fi
		export LIBRARY_PATH=$LD_LIBRARY_PATH
	else										# -D option gets us here to test an installed library
		export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
		export LIBRARY_PATH=$LD_LIBRARY_PATH
	fi
fi

export RMR_SEED_RT=${RMR_SEED_RT:-./ipv6.rt}		# allow easy testing with different rt

if [[ ! -f $RMR_SEED_RT ]]							# create special rt
then
	mk_rt
fi

if (( rebuild || force_make )) || [[ ! -f ./sender${si} || ! -f ./receiver${si} ]]
then
	if ! make -B sender${si} receiver${si} >/tmp/PID$$.log 2>&1
	then
		echo "[FAIL] cannot find sender{$si} and/or receiver${si} binary, and cannot make them.... humm?"
		cat /tmp/PID$$.log
		rm -f /tmp/PID$$.*
		exit 1
	fi
fi

run_rcvr &
sleep 2				# if caller starts faster than rcvr we can drop, so pause a bit
run_sender &

wait
rrc=$(head -1 /tmp/PID$$.rrc)		# get pass/fail state from each
src=$(head -1 /tmp/PID$$.src)

if (( !! (src + rrc) ))
then
	echo "[FAIL] sender rc=$src  receiver rc=$rrc"
else
	echo "[PASS] sender rc=$src  receiver rc=$rrc"
fi

rm /tmp/PID$$.*
rm -f .verbose

exit $(( !! (src + rrc) ))

