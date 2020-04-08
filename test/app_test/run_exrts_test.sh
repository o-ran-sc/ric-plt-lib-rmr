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
#	Mnemonic:	run_exrts_test.ksh
#	Abstract:	This is a simple script to set up and run the v_send/ex_rts_receive
#				processes for some library validation on top of nng. This test
#				starts these processes to verify that when a receiver has an ack 
#				message larger than the received message it is able to allocate
#				a new payload which can be returned via the RMR return to sender
#				function.
#
#				Example command line:
#					# run with 10 caller threads sending 10,000 meessages each, 
#					# 5 receivers, and a 10 mu-s delay between each caller send
#					ksh ./run_lcall_test.ksh -d 10 -n 10000 -r 5 -c 10
#
#	Date:		28 October 2019
#	Author:		E. Scott Daniels
# ---------------------------------------------------------------------------------

ulimit -c unlimited

# driven with -L sender or -L receiver on the command line
# run something though valgrind to check for leaks; requires valgind
function leak_anal {
	valgrind  -v --leak-resolution=high --leak-check=yes $opt "$@"
}

# The sender and receivers are run asynch. Their exit statuses are captured in a
# file in order for the 'main' to pick them up easily.
#
function run_sender {
	if (( la_sender ))
	then
		leak_anal ./v_sender${si} ${nmsg:-10} ${delay:-100000} ${mtype_start_stop:-0:1}  >/tmp/la.log 2>&1
	else
		./v_sender${si} ${nmsg:-10} ${delay:-100000} ${mtype_start_stop:-0:1}
	fi
	echo $? >/tmp/PID$$.src		# must communicate state back via file b/c asynch
}

# $1 is the instance so we can keep logs separate
function run_rcvr {
	typeset port

	port=$(( 4460 + ${1:-0} ))
	export RMR_RTG_SVC=$(( 9990 + $1 ))
	if (( la_receiver ))
	then
		leak_anal ./ex_rts_receiver${si} $copyclone -p $port >/tmp/la.log 2>&1
	else
		./ex_rts_receiver${si} $copyclone -p $port
	fi
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

	cat <<endKat >ex_rts.rt
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

	cat ex_rts.rt
}

# ---------------------------------------------------------

if [[ ! -f local.rt ]]		# we need the real host name in the local.rt; build one from mask if not there
then
	hn=$(hostname)
	sed "s!%%hostname%%!$hn!" rt.mask >local.rt
fi

export EX_CFLAGS=""
export RMR_ASYNC_CONN=0 	# ensure we don't lose first msg as drops waiting for conn look like errors
nmsg=100					# total number of messages to be exchanged (-n value changes)
delay=500					# microsec sleep between msg 1,000,000 == 1s
wait=1
rebuild=0
verbose=0
nrcvrs=1					# this is sane, but -r allows it to be set up
use_installed=0
mtype_start_stop=""
si=""						# -S will enable si library testing

while [[ $1 == -* ]]
do
	case $1 in
		-c)	copyclone="-c $2"; shift;;
		-B)	rebuild=1;;
		-d)	delay=${2//,/}; shift;;				# delay in micro seconds allow 1,000 to make it easier on user
		-i)	use_installed=1;;
		-L)	leak_anal=$2; shift;;
		-m)	mtype_start_stop="$2"; shift;;
		-M)	mt_call_flag="EX_CFLAGS=-DMTC=1";;	# turn on mt-call receiver option
		-N)	si="";;								# enable nng based testing
		-n)	nmsg=$2; shift;;
		-r) nrcvrs=$2; shift;;
		-S)	si="_si";;							# run SI95 based binaries
		-v)	verbose=1;;

		*)	echo "unrecognised option: $1"
			echo "usage: $0 [-B] [-c caller-threads] [-d micor-sec-delay] [-i] [-M] [-m mtype] [-n num-msgs] [-r num-receivers] [-S] [-v]"
			echo "  -B forces a rebuild which will use .build"
			echo "  -i will use installed libraries (/usr/local) and cause -B to be ignored if supplied)"
			echo "  -L {sender|receiver} run the sender or recevier code under valgrind for leak analysis (output to /tmp/la.log)"
			echo "  -m mtype  will set the stopping (max) message type; sender will loop through 0 through mtype-1"
			echo "  -m start:stop  will set the starting and stopping mtypes; start through stop -1"
			echo "  -M enables mt-call receive processing to test the RMR asynch receive pthread"
			echo "  -N enables NNG based testing (default)"
			echo "  -S enables SI95 based testing"
			echo ""
			echo "The receivers will run until they have not received a message for a few seconds. The"
			echo "sender will send the requested number of messages, 10 by default."
			exit 1
			;;
	esac

	shift
done

# set leak analysis (do not do this from any automated tests)
case $leak_anal in 
	s*)	la_sender=1;;
	r*)	la_receiver=1;;
esac

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

	if [[ -z $LD_LIBRARY_PATH ]]		# cmake test will set this up and we must honour
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
export RMR_SEED_RT=./ex_rts.rt

set_rt $nrcvrs														# set up the rt for n receivers

if ! build_path=$build_path make -B $mt_call_flag v_sender${si} ex_rts_receiver${si} >/tmp/PID$$.log 2>&1			# for sanity, always rebuild test binaries
then
	echo "[FAIL] cannot make binaries"
	echo "====="
	env
	echo "====="
	cat /tmp/PID$$.log
	rm -f /tmp/PID$$*
	exit 1
fi

echo "<RUN> binaries built"

for (( i=0; i < nrcvrs; i++  ))
do
	run_rcvr $i &
done

sleep 2				# let receivers init so we don't shoot at empty targets
if (( verbose ))
then
	netstat -an|grep LISTEN
fi
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
	rm -f ex_rts.rt
fi

rm /tmp/PID$$.*
rm -f .verbose

exit $(( !! (src + rrc) ))

