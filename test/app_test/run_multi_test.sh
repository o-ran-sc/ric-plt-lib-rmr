#!/usr/bin/env ksh
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
#	Mnemonic:	run_multi_test.ksh
#	Abstract:	This is a simple script to set up and run the basic send/receive
#				processes for some library validation on top of nng. This
#				particular tests starts several receivers and creates a route table
#				which causes messages to be sent to all receivers in parallel
#				(forcing message cloning internally in RMr).
#				It should be possible to clone the repo, switch to this directory
#				and execute  'ksh run -B'  which will build RMr, make the sender and
#				recevier then  run the basic test.
#
#				Example command line:
#					ksh ./run_multi_test.ksh		# default 10 messages at 1 msg/sec
#					ksh ./run_multi_test.ksh -d 100 -n 10000 # send 10k messages with 100ms delay between
#
#	Date:		24 April 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------


# The sender and receivers are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily. For the multi test the 
# async connect must be DISABLED because in some environments (um, jenkins) the
# session connect time lags enough that the first message can be dropped silently.
# It doesn't happen all of the time, but frequently enough to be annoying. 
#
function run_sender {
	RMR_ASYNC_CONN=0 ./sender $nmsg $delay
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch
}

# $1 is the instance so we can keep logs separate
function run_rcvr {
	typeset port

	port=$(( 4460 + ${1:-0} ))
	export RMR_RTG_SVC=$(( 9990 + $1 ))
	./receiver $nmsg $port
	echo $? >/tmp/PID$$.$1.rrc
}

#	Drop a contrived route table in such that the sender sends each message to n
#	receivers.
#
function set_rt {
	typeset port=4460
	typeset groups="127.0.0.1:4460"
	for (( i=1; i < ${1:-3}; i++ ))
	do
		groups="$groups;127.0.0.1:$((port+i))"
	done

	cat <<endKat >multi.rt
		newrt | start
		mse |0 | 0 | $groups
		mse |1 | 10 | $groups
		mse |2 | 20 | $groups
		rte |3 | $groups
		rte |4 | $groups
		rte |5 | $groups
		rte |6 | $groups
		rte |7 | $groups
		rte |8 | $groups
		rte |9 | $groups
		rte |10 | $groups
		rte |11 | $groups
		newrt | end
endKat

}

# ---------------------------------------------------------

if [[ ! -f local.rt ]]		# we need the real host name in the local.rt; build one from mask if not there
then
	hn=$(hostname)
	sed "s!%%hostname%%!$hn!" rt.mask >local.rt
fi

nmsg=10						# total number of messages to be exchanged (-n value changes)
delay=1000000				# microsec sleep between msg 1,000,000 == 1s
wait=1
rebuild=0
nopull=""
verbose=0
nrcvrs=3					# this is sane, but -r allows it to be set up
force_make=0
si=""

while [[ $1 == -* ]]
do
	case $1 in
		-B)	rebuild=1;;
		-b)	rebuild=1; nopull="nopull";;		# enable build but without pull
		-d)	delay=$2; shift;;
		-n)	nmsg=$2; shift;;
		-N)	;;								# buld/run NNG binaries (turn off si)
		-M)	force_make=1;;
		-r)	nrcvrs=$2; shift;;
		-S)	;;							# buld/run SI95 binaries
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-d micor-sec-delay] [-M] [-n num-msgs] [-S]"
			echo "  -B forces an RMR rebuild which will use .build"
			echo "  -m force test applications to be remade"
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

if [[ -z LD_LIBRARY_PATH ]]			# if cmake test sets we must honour
then
	if [[ -d $build_path/lib64 ]]
	then
		export LD_LIBRARY_PATH=$build_path:$build_path/lib64:$LD_LIBRARY_PATH
	else
		export LD_LIBRARY_PATH=$build_path:$build_path/lib:$LD_LIBRARY_PATH
	fi
fi

export LIBRARY_PATH=$LD_LIBRARY_PATH
export RMR_SEED_RT=./multi.rt

set_rt $nrcvrs						# set up the rt for n receivers

if (( rebuild || force_make )) || [[ ! -f ./sender || ! -f ./receiver ]]
then
	if ! make >/dev/null 2>&1
	then
		echo "[FAIL] cannot find sender and/or receiver binary, and cannot make them.... humm?"
		exit 1
	fi
fi

for (( i=0; i < nrcvrs; i++ ))		# start the receivers with an instance number
do
	run_rcvr $i &
done

sleep 2				# let receivers init so we don't shoot at an empty target
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
	rm -f multi.rt
fi

rm /tmp/PID$$.*
rm -f .verbose

exit $(( !! (src + rrc) ))

