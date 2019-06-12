#!/bin/bash
#==================================================================================
#       Copyright (c) 2019 Nokia 
#       Copyright (c) 2018-2019 AT&T Intellectual Property.
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

set -eux -o pipefail
echo "--> extract-build-artifacts.sh"

# copies build artifacts created by the builder to host

# TODO: pass in docker image name and tag
image="ric-plt-lib-rmr"
tag="latest"
file="/tmp/rmr_deb_path"

container=$(docker run -d $image:$tag ls /tmp)
docker logs "$container"
docker cp "$container:$file" .
filebase=$(basename $file)
deb=$(cat "$filebase")
docker cp "$container:$deb" .
