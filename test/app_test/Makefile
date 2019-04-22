#==================================================================================
#    Copyright (c) 2019 Nokia
#    Copyright (c) 2018-2019 AT&T Intellectual Property.
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

# NOTE:  this makefile assumes that RMr has been built using the directory .build
#		at the top most repo directory (e.g. ../../.build). It can be changed 
#		if you need to by adding "build_path=<path>" to the make command line.

.EXPORT_ALL_VARIABLES:
.ONESHELL:
.SHELLFLAGS = -e
SHELL = /bin/ksh

build_path ?= ../../.build
header_path := $(shell find ../../.build -name 'rmr.h' |head -1 | sed 's!/rmr/.*!!' )

C_INCLUDE_PATH := $(header_path)
LD_LIBRARY_PATH=$(build_path):$(build_path)/lib
LIBRARY_PATH = $(LD_LIBRARY_PATH)

# These programmes are designed to test some basic application level functions
# from the perspective of two communicating processes. 


.PHONY: all
all: sender receiver sender_nano receiver_nano

receiver_nano: receiver.c
	gcc -I $${C_INCLUDE_PATH:-.} $< -g -o $@ -lrmr -lnanomsg -lpthread -lm

receiver: receiver.c
	gcc -I $${C_INCLUDE_PATH:-.} $< -g -o $@ -lrmr_nng -lnng -lpthread -lm

sender_nano: sender.c
	gcc -I $${C_INCLUDE_PATH:-.} $< -g -o $@ -lrmr -lnanomsg -lpthread -lm

sender: sender.c
	gcc -I $${C_INCLUDE_PATH:-.} $< -g -o $@ -lrmr_nng -lnng -lpthread -lm


.PHONY: clean
clean:
	rm -f sender sender_nano receiver receiver_nano *.o
