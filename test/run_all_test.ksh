#!/usr/bin/env ksh

#==================================================================================
#        Copyright (c) 2019 Nokia 
#        Copyright (c) 2018-2019 AT&T Intellectual Property.
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
#	Mnemonic:	run_all_tests.ksh
#	Abstract:	This should allow one stop shopping to run all tests as
#				follows some are explicit, some are implied (e.g. if nng
#				doesn't build correctly RMr won't build):
#
#				- complete build of RMr code and generation of a .deb with 
#				  expected content
#
#				- complete build of Nanomsg and NNG
#
#				- complete execution of the unit test script
#
#				Assumptions:
#					- code is cloned, and the PWD is the top level repo directory
#					- running in a container such that 'make install' can be
#					  executed to install RMr and NNG/Nano libraires in /usr/local
#					- needed utilities (cmake, make, gcc, ksh) are installed
#
#	Date:		17 April 2019
#	Author:		E. Scott Daniels
# -------------------------------------------------------------------------

# cat the file if error or verbose
function err_cat {
	if (( $1 > 0 || verbose ))
	then
		shift
		while [[ -n $1 ]]
		do
			cat $1
			echo ""
			shift
		done
	fi
}

#	if $1 (state) is not 0, write message ($2) and abort
function abort_on_err {
	if (( $1 != 0 ))
	then
		echo "$(date) [FAIL] $2"
		exit 1
	fi
}

function log_it {
	echo "$(date) $1" 
}

verbose=0
refresh=1		# use -R to turn off a git pull before we start

while [[ $1 == -* ]]
do
	case $1 in 
		-R)	refresh=0;;
		-v)	verbose=1;;
	esac

	shift
done

if (( refresh ))
then
	log_it "[INFO] refreshing RMr code base with git pull"
	(
		set -e
		git pull
	) >/tmp/git.log 2>&1
	rc=$?
	err_cat $rc /tmp/git.log
	abort_on_err $rc "unable to refresh RMr code base"
fi

log_it "[INFO] build starts"
# build RMr (and nano/nng)
(
	set -e
	mkdir -p .build
	cd .build
	cmake .. -DBUILD_DOC=1
	make package				# build RMr, then put in the .deb
) >/tmp/build.log 2>&1			# capture the reams of output and show only on error
rc=$?
err_cat $rc /tmp/build.log
abort_on_err $rc "unable to setup cmake or build and install"

log_it "[OK]    Build successful"

log_it "[INFO] validating .deb"
(
	set -e
	cd .build
	ls -al *.deb
	if whence dpkg >/dev/null 2>&1
	then
		dpkg -i *.deb
	else
		log_it "[INFO]   Deb installation check skipped. dpkg does not exist; trying make install"
		make install
	fi
) >/tmp/dpkg.log 2>&1
rc=$?
err_cat $rc /tmp/dpkg.log
abort_on_err $rc "unable to install from .deb"

log_it "[OK]   Deb installation successful"

PATH=$PATH:.
export LD_LIBRARY_PATH=/usr/local/lib
export C_INCLUDE_PATH=../.build/include			# must reference nano/nng from the build tree

log_it "[INFO] unit testing starts"
(
	set -e
	cd test
	pwd
	ls -al unit_test.ksh
	./unit_test.ksh
) >/tmp/utest.log 2>&1
rc=$?
err_cat $rc /tmp/utest.log
abort_on_err $rc "unit tests failed"
log_it "[OK]   unit testing passes"

echo ""
log_it "[PASS]  all testing successful"

