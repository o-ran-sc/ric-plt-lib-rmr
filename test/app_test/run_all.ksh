
# run all of the tests, building rmr before the first one if -B is on the command line.

build=""

while [[ $1 == "-"* ]]
do
	case $1 in 
		-B)	build="-B";;
		-i)	installed="-i";;

		*)	echo "'$1' is not a recognised option and is ignored";;
	esac

	shift
done

set -e
echo "---- app -------------"
ksh run_app_test.ksh -v $installed $build
echo "----- multi -----------"
ksh run_multi_test.ksh
echo "----- round robin ----"
ksh run_rr_test.ksh
echo "----- rts ------------"
ksh run_rts_test.ksh -s 20
