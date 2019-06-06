
# run all of the tests, building rmr before the first one if -B is on the command line.

build=""

while [[ $1 == "-"* ]]
do
	case $1 in 
		-B)	build="-B";;

		*)	echo "'$1' is not a recognised option and is ignored";;
	esac

	shift
done

set -e
ksh run_app_test.ksh -v -i $build
ksh run_multi_test.ksh
ksh run_rr_test.ksh
ksh run_rts_test.ksh -s 20
