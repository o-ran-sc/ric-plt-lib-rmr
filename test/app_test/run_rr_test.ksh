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
#	Mnemonic:	run_multi_test.ksh
#	Abstract:	This is a simple script to set up and run the basic send/receive
#				processes for some library validation on top of nng. This
#				particular tests starts several receivers and creates a route table
#				which causes messages to be sent round robin to all of the receivers.
#				The number of messages command line parameter (-n) will be the number
#				of messages that each receiver should expect; the sender will be asked
#				to send r times that many messages so that as they are round robbined
#				each receiver should get the same number of messages.
#
#				Example command line:
#					ksh ./run_rr_test.ksh		# default 10 messages at 1 msg/sec
#					ksh ./run_rr_test.ksh -d 100 -n 10000 # send 10k messages with 100ms delay between
#
#	Date:		24 April 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------


# The sender and receivers are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily.
#
function run_sender {
	export RMR_RTG_SVC=8990
	./sender${si} $(( nmsg * nrcvrs ))  $delay $max_mtype
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch
}

# $1 is the instance so we can keep logs separate
function run_rcvr {
	typeset port

	port=$(( 4560 + ${1:-0} ))
	export RMR_RTG_SVC=$(( 9990 + $1 ))
	./receiver${si} $nmsg $port
	echo $? >/tmp/PID$$.$1.rrc
}

#
#	Drop a contrived route table in such that the sender sends each message to n
#	receivers.
#
function set_rt {
	typeset port=4560
	typeset endpoints="localhost:4560"
	for (( i=1; i < ${1:-3}; i++ ))
	do
		endpoints="$endpoints,localhost:$((port+i))"
	done

	cat <<endKat >rr.rt
		newrt |start
		rte |0 | $endpoints  |0
		rte |1 | $endpoints  |10
		mse |2 | 20 | $endpoints		# new style mtype/subid entry
		rte |3 | $endpoints  | -1
		rte |4 | $endpoints  | -1
		rte |5 | $endpoints  | -1
		rte |6 | $endpoints  | -1
		rte |7 | $endpoints  | -1
		rte |8 | $endpoints  | -1
		rte |9 | $endpoints  | -1
		rte |10 | $endpoints  | -1
		rte |11 | $endpoints  | -1
		newrt |end
endKat

}

# ---------------------------------------------------------

if [[ ! -f local.rt ]]		# we need the real host name in the local.rt; build one from mask if not there
then
	hn=$(hostname)
	sed "s!%%hostname%%!$hn!" rt.mask >local.rt
fi

export RMR_ASYNC_CONN=0 	# ensure we don't lose first msg as drops waiting for conn look like errors
nmsg=10						# total number of messages to be exchanged (-n value changes)
delay=1000					# microsec sleep between msg 1,000,000 == 1s (shorter than others b/c/ we are sending to multiple)
wait=1
rebuild=0
nopull=""
verbose=0
max_mtype=1					# causes all msgs to go with type 1; use -M to set up, but likely harder to validate
nrcvrs=3					# this is sane, but -r allows it to be set up
force_make=0
si=""

while [[ $1 == -* ]]
do
	case $1 in
		-B)	rebuild=1;;
		-b)	rebuild=1; nopull="nopull";;		# build without pulling
		-d)	delay=$2; shift;;
		-m) max_mtype=$2; shift;;
		-M) force_make=1;;
		-n)	nmsg=$2; shift;;
		-N)	si="";;								# build/run NNG binaries
		-r)	nrcvrs=$2; shift;;
		-S)	si="_si";;							# build/run SI95 binaries
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-d micor-sec-delay] [-M] [-n num-msgs] [-S]"
			echo "  -B forces a rebuild which will use .build"
			echo "  -M force test applications to be remade"
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
	set -e
	$SHELL ./rebuild.ksh $nopull | read build_path
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

if [[ -z $LD_LIBRARY_PATH ]]			# honour if cmake test sets
then
	if [[ -d $build_path/lib64 ]]
	then
		export LD_LIBRARY_PATH=$build_path:$build_path/lib64:$LD_LIBRARY_PATH
	else
		export LD_LIBRARY_PATH=$build_path:$build_path/lib:$LD_LIBRARY_PATH
	fi
fi

export LIBRARY_PATH=$LD_LIBRARY_PATH
export RMR_SEED_RT=./rr.rt

set_rt $nrcvrs

if (( rebuild || force_make )) || [[ ! -f ./sender${si} || ! -f ./receiver${si} ]]
then
	if ! make >/dev/null 2>&1
	then
		echo "[FAIL] cannot find sender${si} and/or receiver${si} binary, and cannot make them.... humm?"
		exit 1
	fi
fi

for (( i=0; i < nrcvrs; i++ ))		# start the receivers with an instance number
do
	run_rcvr $i &
done

sleep 2					# wait to start sender else we might send before receivers up and drop messages
run_sender &

wait


for (( i=0; i < nrcvrs; i++ ))		# collect return codes
do
	head -1 /tmp/PID$$.$i.rrc | read x
	(( rrc += x ))
done

head -1 /tmp/PID$$.src | read src

if (( !! (src + rrc) ))
then
	echo "[FAIL] sender rc=$src  receiver rc=$rrc"
else
	echo "[PASS] sender rc=$src  receiver rc=$rrc"
fi

rm /tmp/PID$$.*
rm -f .verbose
rm -f rr.rt

exit $(( !! (src + rrc) ))

