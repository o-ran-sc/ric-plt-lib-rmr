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

# CI to verify the RMR library and build run-time and dev packages
# Inherits C toolchain from buildpack-deps:stretch
# Adds cmake ksh and alien for RMR

FROM buildpack-deps:stretch
RUN apt-get update && apt-get -q -y install cmake ksh alien
ADD . /tmp
WORKDIR /tmp

# build RMr, run unit tests, and generate packages
RUN ksh ci/ci_build.ksh

