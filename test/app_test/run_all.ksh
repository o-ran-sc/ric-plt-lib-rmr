
# run all of the tests, building rmr before the first one.
set -e
ksh run_app_test.ksh -B
ksh run_multi_test.ksh
ksh run_rr_test.ksh
ksh run_rts_test.ksh -s 20
