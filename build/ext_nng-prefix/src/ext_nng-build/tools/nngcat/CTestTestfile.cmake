# CMake generated Testfile for 
# Source directory: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat
# Build directory: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(nngcat_async "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_async_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_async PROPERTIES  TIMEOUT "10")
add_test(nngcat_ambiguous "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_ambiguous_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_ambiguous PROPERTIES  TIMEOUT "2")
add_test(nngcat_need_proto "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_need_proto_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_need_proto PROPERTIES  TIMEOUT "2")
add_test(nngcat_dup_proto "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_dup_proto_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_dup_proto PROPERTIES  TIMEOUT "2")
add_test(nngcat_help "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_help_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_help PROPERTIES  TIMEOUT "2")
add_test(nngcat_incompat "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_incompat_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_incompat PROPERTIES  TIMEOUT "2")
add_test(nngcat_pubsub "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_pubsub_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_pubsub PROPERTIES  TIMEOUT "20")
add_test(nngcat_recvmaxsz "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_recvmaxsz_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_recvmaxsz PROPERTIES  TIMEOUT "20")
add_test(nngcat_unlimited "/bin/bash" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tools/nngcat/nngcat_unlimited_test.sh" "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tools/nngcat/nngcat")
set_tests_properties(nngcat_unlimited PROPERTIES  TIMEOUT "20")
