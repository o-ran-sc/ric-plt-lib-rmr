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
#	Mnemonic:	run_lcall_test.ksh
#	Abstract:	This is a simple script to set up and run the basic send/receive
#				processes for some library validation on top of nng. This
#				particular test starts the latency caller and latency receiver
#				processes such that they exchange messages and track the latency
#				from the caller's perepective (both outbound to receiver, and then
#				back.  Stats are presented at the end.   This test is NOT intended
#				to be used as a CI validation test.
#
#				The sender and receiver processes all have a 20s timeout (+/-)
#				which means that all messages must be sent, and acked within that
#				time or the processes will give up and report failure.  Keep in mind
#				that n messages with a delay value (-d) set will affect whether or
#				not the messages can be sent in the 20s timeout period.  There is
#				currently no provision to adjust the timeout other than by changing
#				the C source.  The default (100 msgs with 500 micro-sec delay) works
#				just fine for base testing.
#
#				Example command line:
#					# run with 10 caller threads sending 10,000 meessages each, 
#					# 5 receivers, and a 10 mu-s delay between each caller send
#					ksh ./run_lcall_test.ksh -d 10 -n 10000 -r 5 -c 10
#
#	Date:		28 May 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------


# The sender and receivers are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily.
#
function run_sender {
	./lcaller${si} ${nmsg:-10} ${delay:-500} ${cthreads:-3} 
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch
}

# $1 is the instance so we can keep logs separate
function run_rcvr {
	typeset port

	port=$(( 4460 + ${1:-0} ))
	export RMR_RTG_SVC=$(( 9990 + $1 ))
	./lreceiver${si} $(( ((nmsg * cthreads)/nrcvrs) + 10 )) $port
	echo $? >/tmp/PID$$.$1.rrc
}

#	Drop a contrived route table in such that the sender sends each message to n
#	receivers.
#
function set_rt {
	typeset port=4460
	typeset groups="localhost:4460"
	for (( i=1; i < ${1:-3}; i++ ))
	do
		groups="$groups,localhost:$((port+i))"
	done

	cat <<endKat >lcall.rt
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

export RMR_ASYNC_CONN=0 	# ensure we don't lose first msg as drops waiting for conn look like errors
cthreads=3					# number of caller threads
nmsg=100					# total number of messages to be exchanged (-n value changes)
delay=500					# microsec sleep between msg 1,000,000 == 1s
wait=1
rebuild=0
verbose=0
nrcvrs=3					# this is sane, but -r allows it to be set up
use_installed=0
force_make=0
si=""

while [[ $1 == -* ]]
do
	case $1 in
		-c)	cthreads=$2; shift;;
		-B)	rebuild=1;;
		-d)	delay=$2; shift;;
		-i)	use_installed=1;;
		-n)	nmsg=$2; shift;;
		-M)	force_make=1;;
		-N)	si="";;					# build NNG binaries (turn off si)
		-r)	nrcvrs=$2; shift;;
		-S)	si="_si";;				# build SI95 binaries
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-c caller-threads] [-d micor-sec-delay] [-i] [-M] [-n num-msgs] [-r num-receivers] [-S]"
			echo "  -B forces an RMR rebuild which will use .build"
			echo "  -i will use installed libraries (/usr/local) and cause -B to be ignored if supplied)"
			echo "  -M force test applictions to be remade"
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

if (( use_installed ))			# point at installed library
then
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH-:/usr/local/lib}	# use caller's or set sane default
else
	if (( rebuild ))
	then
		build_path=../../.build		# if we rebuild we can insist that it is in .build :)
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

	if [[ -z $LD_LIBRARY_PATH ]]			# cmake test will set this; we must honour it
	then
		if [[ -d $build_path/lib64 ]]
		then
			export LD_LIBRARY_PATH=$build_path:$build_path/lib64:$LD_LIBRARY_PATH
		else
			export LD_LIBRARY_PATH=$build_path:$build_path/lib:$LD_LIBRARY_PATH
		fi
	fi
fi

export LIBRARY_PATH=$LD_LIBRARY_PATH
export RMR_SEED_RT=./lcall.rt

set_rt $nrcvrs						# set up the rt for n receivers

if (( force_make || rebuild )) || [[ ! -f ./lcaller{$si} || ! -f ./lreceiver${si} ]]
then
	if ! make -B lcaller${si} lreceiver${si} >/dev/null 2>&1
	then
		echo "[FAIL] cannot find lcaller${si} and/or lreceiver${si} binary, and cannot make them.... humm?"
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
	rm -f lcall.rt
fi

rm /tmp/PID$$.*
rm -f .verbose

exit $(( !! (src + rrc) ))

