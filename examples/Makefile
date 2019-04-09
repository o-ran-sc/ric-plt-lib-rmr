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

# The demo programmes assume that RMr (along with nng) is installed
# It may be necessary to set LD_LIBRARY_PATH=/usr/local/lib


.PHONY: all
all: sender receiver

receiver: receiver.c
	gcc $< -g -o $@ -lrmr_nng -lnng -lpthread -lm

sender: sender.c
	gcc $< -g -o $@ -lrmr_nng -lnng -lpthread -lm


