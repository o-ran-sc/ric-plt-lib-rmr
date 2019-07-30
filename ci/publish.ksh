# -----------------------------------------------------------------------------
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
#
# -----------------------------------------------------------------------------

#	Mnemonic:	publish
#	Abstract:	Simple script which reads a list of files from
#				/tmp/build_packages.txt and copies the files to a data
#				directory.  The data directory is assumed to be mounted
#				from the outside world as /data, though we will use $1
#				as an override so this can be changed if needed.
#
#	Date:		30 July 2019
#
# -----------------------------------------------------------------------------

target=${1:-/data}
pkg_list=/tmp/build_packages.txt		# where the build script left the list

if ! cd $target
then
	echo "abort: cannot find or switch to: $target" >&2
	exit 1
fi

if [[ ! -f $pkg_list ]]
then
	echo "abort: unable to find the list of packages: $pkg_list" >&2
	exit 1
fi

errors=0
while read f				# copy all things from the package list
do
	if ! cp $f $target/
	then
		(( errors++ ))
	fi
done <$pkg_list


exit $(( !! $errors ))		# ensure exit code is !0 if there are errors
