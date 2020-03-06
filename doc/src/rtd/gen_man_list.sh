#/usr/bin/env bash

# generate the list of man pages to include in the user guide
ls -al ../man/*.xfm|sed 's!^.*man/!!' | while read x
do
	if [[ $x != *".7.xfm" ]]
	then
		printf ".ju off &space\n"
		printf ".im &{lib}/man/$x\n"
	fi
done
