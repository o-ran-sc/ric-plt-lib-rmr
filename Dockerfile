# O-RAN-SC
#
# Copyright (C) 2019 AT&T Intellectual Property and Nokia
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# CI to verify the RMR library
# Inherits C toolchain from buildpack-deps:stretch
# Adds cmake for RMR

FROM buildpack-deps:stretch
RUN apt-get update && apt-get -q -y install cmake ksh
ADD . /tmp
# tests require directory named ".build"
RUN cd /tmp && mkdir .build && cd .build && cmake .. && make install
RUN cd /tmp/test && ksh unit_test.ksh -v
RUN cd /tmp/test/app_test && ksh run_all.ksh
