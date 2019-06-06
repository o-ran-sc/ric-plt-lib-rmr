#!/usr/bin/env ksh
# :vi ts=4 sw=4 noet :
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
#	Mnemonic:	run_app_test.ksh
#	Abstract:	This is a simple script to set up and run the basic send/receive
#				processes for some library validation on top of nano/nng.
#				It should be possible to clone the repo, switch to this directory
#				and execute  'ksh run -B'  which will build RMr, make the sender and
#				recevier then  run the basic test.
#
#				Example command line:
#					ksh ./run_app_test.ksh		# default 20 messages at 2 msg/sec
#					ksh ./run_app_test.ksh -N    # default but with nanomsg lib
#					ksh ./run_app_test.ksh -d 100 -n 10000 # send 10k messages with 100ms delay between
#
#	Date:		22 April 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------


# The sender and receiver are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily.
#
function run_sender {
	if (( $nano_sender ))
	then
		./sender_nano $nmsg $delay
	else
		./sender $nmsg $delay
	fi
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch
}

function run_rcvr {
	if (( $nano_receiver ))
	then
		./receiver_nano $nmsg
	else
		./receiver $nmsg
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
			mse | 0 |  0 | localhost:$port,$my_ip:43086
			mse | 1 | 10 | localhost:$port,${my_host//.*/}:43086
			mse | 2 | 20 | localhost:$port
			rte | 3 | localhost:$port
			mse | 3 | 100 | localhost:$port	# special test to ensure that this does not affect previous entry
			rte | 4 | localhost:$port
			rte | 5 | localhost:$port
			rte | 6 | localhost:$port
			rte | 7 | localhost:$port
			rte | 8 | localhost:$port
			rte | 9 | localhost:$port
			rte | 10 | localhost:$port
			rte | 11 | localhost:$port
			rte | 12 | localhost:$port
			rte | 13 | localhost:$port
		newrt | end

head -3 app_test.rt

endKat

}

# ---------------------------------------------------------

nmsg=20						# total number of messages to be exchanged (-n value changes)
							# need two sent to each receiver to ensure hairpin entries were removed (will fail if they were not)
delay=500000				# microsec sleep between msg 1,000,000 == 1s
nano_sender=0				# start nano version if set (-N)
nano_receiver=0
wait=1
rebuild=0
nopull=""					# -b sets so that build does not pull
verbose=0
use_installed=0
my_ip=$(snarf_ip)			# get an ip to insert into the route table
keep=0


while [[ $1 == -* ]]
do
	case $1 in
		-B)	rebuild=1;;						# build with pull first
		-b)	rebuild=1; nopull="nopull";;	# buld without pull
		-d)	delay=$2; shift;;
		-k) keep=1;;
		-i) use_installed=1;;
		-N)	nano_sender=1
			nano_receiver=1
			;;
		-n)	nmsg=$2; shift;;
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-d micor-sec-delay] [-i] [-k] [-N] [-n num-msgs]"
			echo "  -B forces a rebuild which will use .build"
			echo "  -i causes the installd libraries (/usr/local) to be referenced; -B is ignored if supplied"
			echo "  -k keeps the route table"
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
	echo "2" >.verbose
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
		ksh ./rebuild.ksh $nopull | read build_path
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

	if [[ -d $build_path/lib64 ]]
	then
		export LD_LIBRARY_PATH=$build_path:$build_path/lib64
	else
		export LD_LIBRARY_PATH=$build_path:$build_path/lib
	fi
	export LIBRARY_PATH=$LD_LIBRARY_PATH
fi

export RMR_SEED_RT=${RMR_SEED_RT:-./app_test.rt}		# allow easy testing with different rt

if [[ ! -f ./sender ]]
then
	if ! make >/dev/null 2>&1
	then
		echo "[FAIL] cannot find sender binary, and cannot make it.... humm?"
		exit 1
	fi
fi

run_rcvr &
sleep 2				# if sender starts faster than rcvr we can drop msgs, so pause a bit
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
rm -f .verbose

exit $(( !! (src + rrc) ))

