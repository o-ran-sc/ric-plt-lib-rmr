# CMake generated Testfile for 
# Source directory: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/perf
# Build directory: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/perf
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(inproc_lat "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/perf/inproc_lat" "64" "10000")
set_tests_properties(inproc_lat PROPERTIES  TIMEOUT "30")
add_test(inproc_thr "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/perf/inproc_thr" "1400" "10000")
set_tests_properties(inproc_thr PROPERTIES  TIMEOUT "30")
