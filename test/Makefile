# :vi ts=4 sw=4 noet:
#
#==================================================================================
#	Copyright (c) 2019 Nokia
#	Copyright (c) 2018-2019 AT&T Intellectual Property.
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


CC = gcc
coverage_opts = -ftest-coverage -fprofile-arcs

#libs = ../build/librmr_nng.a -L ../build/lib -lnng -lpthread -lm
libs =  -L ../build/lib -lnng -lpthread -lm
ipaths = -I ../src/rmr/common/src/ -I ../src/rmr/common/include -I ../src/rmr/nng/include/ -I ../src/rmr/nng/src/ -I ../src/rmr/nanomsg/include/ -I ../src/rmr/nanomsg/src/ 

#sa_tests = sa_tools_test.o

%.o:: %.c
	$(CC) -g $< -c

%:: %.c
	$(CC)  $(ipaths) $(coverage_opts) -fPIC -g $< -o $@  $(libs)

# catch all
all:
	echo "run unit_test.ksh to make and run things here"


# remove intermediates
clean:
	rm -f *.gcov *.gcda *.dcov *.gcno

# remove anything that can be built
nuke: clean
	rm -f ring_test symtab_test
