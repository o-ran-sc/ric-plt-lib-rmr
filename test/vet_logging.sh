#!/usr/bin/env bash

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

#	Mnemonic:	vet_logging.sh
#	Abstract:	Drive the loging test and count the messages we see, and look for
#				messages we don't expect.  Exit good (0)  if all looks good.
#	Date:		16 September 2020
#	Author:		E. Scott Daniels
#-----------------------------------------------------------------------------------

echo "<INFO> log message vetting starts..."

expect=14			# number of good "should be written" messages expected

set -x
logging_test >/tmp/PID$$.out 2>&1
g=$( grep -c "should be written" /tmp/PID$$.out )
b=$( grep -c "should not be written" /tmp/PID$$.out )
set +x

errors=0
if (( b != 0 ))
then
	echo "<FAIL> logging test produced $b unexpected to stderr"
	errors=1
else
	echo "<INFO> no unexpected messages were found"
fi

if (( g != expect ))
then
	echo "<FAIL> logging test did not produce the expected number of messages to stderr"
	echo "<INFO> expected $expect, saw $g"
	errors=1
else
	echo "<INFO> logging test produced good message count ($g)"
fi

if (( errors ))
then
	ls -al /tmp/PID$$.out
	echo "<INFO> --- test output ----"
	cat /tmp/PID$$.out
	echo "<INFO> ---------------------"
else
	echo "<PASS> logging tests pass message output validation"
fi

rm -f /tmp/PID$$.*
exit $(( !! errors ))
