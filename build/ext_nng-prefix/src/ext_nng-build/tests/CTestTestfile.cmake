# CMake generated Testfile for 
# Source directory: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests
# Build directory: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(aio "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/aio" "-v" "-p" "TEST_PORT=13000")
set_tests_properties(aio PROPERTIES  TIMEOUT "5")
add_test(bufsz "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/bufsz" "-v" "-p" "TEST_PORT=13020")
set_tests_properties(bufsz PROPERTIES  TIMEOUT "5")
add_test(device "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/device" "-v" "-p" "TEST_PORT=13040")
set_tests_properties(device PROPERTIES  TIMEOUT "5")
add_test(errors "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/errors" "-v" "-p" "TEST_PORT=13060")
set_tests_properties(errors PROPERTIES  TIMEOUT "2")
add_test(httpclient "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/httpclient" "-v" "-p" "TEST_PORT=13080")
set_tests_properties(httpclient PROPERTIES  TIMEOUT "60")
add_test(inproc "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/inproc" "-v" "-p" "TEST_PORT=13100")
set_tests_properties(inproc PROPERTIES  TIMEOUT "5")
add_test(ipc "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/ipc" "-v" "-p" "TEST_PORT=13120")
set_tests_properties(ipc PROPERTIES  TIMEOUT "5")
add_test(ipcperms "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/ipcperms" "-v" "-p" "TEST_PORT=13140")
set_tests_properties(ipcperms PROPERTIES  TIMEOUT "5")
add_test(ipcsupp "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/ipcsupp" "-v" "-p" "TEST_PORT=13160")
set_tests_properties(ipcsupp PROPERTIES  TIMEOUT "10")
add_test(ipcwinsec "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/ipcwinsec" "-v" "-p" "TEST_PORT=13180")
set_tests_properties(ipcwinsec PROPERTIES  TIMEOUT "5")
add_test(message "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/message" "-v" "-p" "TEST_PORT=13200")
set_tests_properties(message PROPERTIES  TIMEOUT "5")
add_test(multistress "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/multistress" "-v" "-p" "TEST_PORT=13220")
set_tests_properties(multistress PROPERTIES  TIMEOUT "60")
add_test(nonblock "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/nonblock" "-v" "-p" "TEST_PORT=13240")
set_tests_properties(nonblock PROPERTIES  TIMEOUT "60")
add_test(options "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/options" "-v" "-p" "TEST_PORT=13260")
set_tests_properties(options PROPERTIES  TIMEOUT "5")
add_test(pipe "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/pipe" "-v" "-p" "TEST_PORT=13280")
set_tests_properties(pipe PROPERTIES  TIMEOUT "5")
add_test(platform "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/platform" "-v" "-p" "TEST_PORT=13300")
set_tests_properties(platform PROPERTIES  TIMEOUT "5")
add_test(pollfd "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/pollfd" "-v" "-p" "TEST_PORT=13320")
set_tests_properties(pollfd PROPERTIES  TIMEOUT "5")
add_test(reconnect "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/reconnect" "-v" "-p" "TEST_PORT=13340")
set_tests_properties(reconnect PROPERTIES  TIMEOUT "5")
add_test(scalability "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/scalability" "-v" "-p" "TEST_PORT=13360")
set_tests_properties(scalability PROPERTIES  TIMEOUT "20")
add_test(set_recvmaxsize "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/set_recvmaxsize" "-v" "-p" "TEST_PORT=13380")
set_tests_properties(set_recvmaxsize PROPERTIES  TIMEOUT "2")
add_test(sock "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/sock" "-v" "-p" "TEST_PORT=13400")
set_tests_properties(sock PROPERTIES  TIMEOUT "5")
add_test(stats "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/stats" "-v" "-p" "TEST_PORT=13420")
set_tests_properties(stats PROPERTIES  TIMEOUT "5")
add_test(tcpsupp "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/tcpsupp" "-v" "-p" "TEST_PORT=13440")
set_tests_properties(tcpsupp PROPERTIES  TIMEOUT "10")
add_test(tcp "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/tcp" "-v" "-p" "TEST_PORT=13460")
set_tests_properties(tcp PROPERTIES  TIMEOUT "180")
add_test(url "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/url" "-v" "-p" "TEST_PORT=13480")
set_tests_properties(url PROPERTIES  TIMEOUT "5")
add_test(ws "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/ws" "-v" "-p" "TEST_PORT=13500")
set_tests_properties(ws PROPERTIES  TIMEOUT "30")
add_test(wsstream "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/wsstream" "-v" "-p" "TEST_PORT=13520")
set_tests_properties(wsstream PROPERTIES  TIMEOUT "10")
add_test(bus "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/bus" "-v" "-p" "TEST_PORT=13540")
set_tests_properties(bus PROPERTIES  TIMEOUT "5")
add_test(pipeline "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/pipeline" "-v" "-p" "TEST_PORT=13560")
set_tests_properties(pipeline PROPERTIES  TIMEOUT "5")
add_test(pair1 "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/pair1" "-v" "-p" "TEST_PORT=13580")
set_tests_properties(pair1 PROPERTIES  TIMEOUT "5")
add_test(pubsub "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/pubsub" "-v" "-p" "TEST_PORT=13600")
set_tests_properties(pubsub PROPERTIES  TIMEOUT "5")
add_test(reqctx "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/reqctx" "-v" "-p" "TEST_PORT=13620")
set_tests_properties(reqctx PROPERTIES  TIMEOUT "5")
add_test(reqpoll "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/reqpoll" "-v" "-p" "TEST_PORT=13640")
set_tests_properties(reqpoll PROPERTIES  TIMEOUT "5")
add_test(reqrep "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/reqrep" "-v" "-p" "TEST_PORT=13660")
set_tests_properties(reqrep PROPERTIES  TIMEOUT "5")
add_test(reqstress "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/reqstress" "-v" "-p" "TEST_PORT=13680")
set_tests_properties(reqstress PROPERTIES  TIMEOUT "60")
add_test(respondpoll "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/respondpoll" "-v" "-p" "TEST_PORT=13700")
set_tests_properties(respondpoll PROPERTIES  TIMEOUT "5")
add_test(survey "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/survey" "-v" "-p" "TEST_PORT=13720")
set_tests_properties(survey PROPERTIES  TIMEOUT "5")
add_test(surveyctx "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/surveyctx" "-v" "-p" "TEST_PORT=13740")
set_tests_properties(surveyctx PROPERTIES  TIMEOUT "5")
add_test(surveypoll "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/surveypoll" "-v" "-p" "TEST_PORT=13760")
set_tests_properties(surveypoll PROPERTIES  TIMEOUT "5")
add_test(compat_block "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_block" "13780")
set_tests_properties(compat_block PROPERTIES  TIMEOUT "10")
add_test(compat_bug777 "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_bug777" "13800")
set_tests_properties(compat_bug777 PROPERTIES  TIMEOUT "10")
add_test(compat_bus "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_bus" "13820")
set_tests_properties(compat_bus PROPERTIES  TIMEOUT "10")
add_test(compat_cmsg "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_cmsg" "13840")
set_tests_properties(compat_cmsg PROPERTIES  TIMEOUT "10")
add_test(compat_msg "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_msg" "13860")
set_tests_properties(compat_msg PROPERTIES  TIMEOUT "10")
add_test(compat_iovec "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_iovec" "13880")
set_tests_properties(compat_iovec PROPERTIES  TIMEOUT "10")
add_test(compat_device "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_device" "13900")
set_tests_properties(compat_device PROPERTIES  TIMEOUT "10")
add_test(compat_pair "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_pair" "13920")
set_tests_properties(compat_pair PROPERTIES  TIMEOUT "10")
add_test(compat_pipeline "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_pipeline" "13940")
set_tests_properties(compat_pipeline PROPERTIES  TIMEOUT "10")
add_test(compat_poll "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_poll" "13960")
set_tests_properties(compat_poll PROPERTIES  TIMEOUT "10")
add_test(compat_reqrep "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_reqrep" "13980")
set_tests_properties(compat_reqrep PROPERTIES  TIMEOUT "10")
add_test(compat_survey "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_survey" "14000")
set_tests_properties(compat_survey PROPERTIES  TIMEOUT "10")
add_test(compat_reqttl "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_reqttl" "14020")
set_tests_properties(compat_reqttl PROPERTIES  TIMEOUT "10")
add_test(compat_shutdown "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_shutdown" "14040")
set_tests_properties(compat_shutdown PROPERTIES  TIMEOUT "10")
add_test(compat_surveyttl "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_surveyttl" "14060")
set_tests_properties(compat_surveyttl PROPERTIES  TIMEOUT "10")
add_test(compat_tcp "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_tcp" "14080")
set_tests_properties(compat_tcp PROPERTIES  TIMEOUT "60")
add_test(compat_ws "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_ws" "14100")
set_tests_properties(compat_ws PROPERTIES  TIMEOUT "60")
add_test(compat_options "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/compat_options" "-v" "-p" "TEST_PORT=14120")
set_tests_properties(compat_options PROPERTIES  TIMEOUT "5")
add_test(cplusplus_pair "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/cplusplus_pair")
set_tests_properties(cplusplus_pair PROPERTIES  TIMEOUT "5")
