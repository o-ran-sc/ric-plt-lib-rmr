#!/usr/bin/env bash

#	Drive the loging test and count the messages we see, and look for
#	messages we don't expect.  Exit good (0)  if all looks good.

echo "<INFO> log message vetting starts..."

expect=13			# number of good "should be written" messages expected

logging_test >/tmp/PID$$.out 2>&1
g=$( grep -c "should be written" /tmp/PID$$.out )
b=$( grep -c "sould not be written" /tmp/PID$$.out )

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
	echo "<INFO> logging test produced good message count"
fi

if (( errorrs ))
then
	cat /tmp/PID$$.out
else
	echo "<PASS> logging tests pass message output validation"
fi

rm -f /tmp/PID$$.*
exit $(( !! errors ))
