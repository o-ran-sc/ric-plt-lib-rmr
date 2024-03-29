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

libs = -lpthread -lm
ipaths = -I ../src/rmr/common/src/ -I ../src/rmr/common/include  \
	-I ../src/rmr/si/include -I ../src/rmr/si/src -I ../src/rmr/si/si95

#sa_tests = sa_tools_test.o

%.o:: %.c
	$(CC) -g $< -c

%:: %.c
	$(CC)  $(ipaths) $(coverage_opts) -fPIC -g $< -o $@  $(libs)

# run all tests; generates .gcov and .dcov files.
all:
	bash unit_test.ksh

# a generic rule driven from the unit_test script to run all stand alone
# vetters after the build
vet: logging_vet

logging_vet:: logging_test always
	bash vet_logging.sh

# Make required hack to always force something to build
always ::


# remove intermediates
clean:
	rm -f *.gcov *.gcda *.dcov *.gcno *.stash*

# remove anything that can be built
nuke: clean
	rm -f ring_test symtab_test logging_test mbuf_api_test rmr_debug_si_test rmr_si_rcv_test rmr_si_test si95_test tools_test
