#
#=================================================================================
#    Copyright (c) 2019-2020 Nokia
#    Copyright (c) 2018-2020 AT&T Intellectual Property.
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

# NOTE:  this makefile assumes that RMR has been built using the directory .build
#		at the top most repo directory (e.g. ../../.build). It can be changed 
#		if you need to by adding "build_path=<path>" to the make command line.
#		To use this makefile to build on a system where RMR is already installed
#		try:    make build_path=/usr/local/lib
#
#		By default we prefer the Korn shell (it's just better). If you really need
#		to build with a dfferent shell add "SHELL=path" to the command line:
#				make SHELL=/bin/bash
#

.EXPORT_ALL_VARIABLES:
.ONESHELL:
#.SHELLFLAGS = -e	# hosed on some flavours so keep it off
SHELL ?= /bin/ksh

ex_cflags = $(shell echo $$EX_CFLAGS )

build_path ?= ../../.build
header_path := $(shell find $(build_path) -name 'rmr.h' |head -1 | sed 's!/rmr/.*!!' )

# use from environment if there so as to allow build to reference installed base
C_INCLUDE_PATH ?= $(header_path)
LD_LIBRARY_PATH ?= $(build_path):$(build_path)/lib
LIBRARY_PATH = $(LD_LIBRARY_PATH)

# These programmes are designed to test some basic application level functions
# from the perspective of two, or more, communicating processes. 


.PHONY: all all_si
all: sender receiver caller mt_receiver v_sender ex_rts_receiver

all_si: sender_si receiver_si


# ------ nng based builds ------------------------------------------------------------

%:%.c
	gcc -I $${C_INCLUDE_PATH:-.} $< -g -o $@ -lrmr_nng -lnng -lpthread -lm

# special build as it reuses an existing source module
mt_receiver: receiver.c
	gcc -I $${C_INCLUDE_PATH:-.} -DMTC $< -g -o $@ -lrmr_nng -lnng -lpthread -lm


# ----- si test builds -------------------------------------------------------------

%_si : %.c
	gcc -I $${C_INCLUDE_PATH:-.} $< -g -o $@ -lrmr_si -lpthread -lm

# explicit rule because there is no mt_recever.c
mt_receiver_si: receiver.c
	gcc -I $${C_INCLUDE_PATH:-.} -DMTC $< -g -o $@ -lrmr_si -lpthread -lm


# --------- housekeeping -----------------------------------------------------------

# clean removes intermediates; nuke removes everything that can be built
.PHONY: clean nuke
clean:
	rm -f *.o

nuke: clean
	rm -f sender receiver caller mt_receiver
