#/usr/bin/env bash
# vim: sw=4 ts=4 noet :

#==================================================================================
#	Copyright (c) 2020 Nokia
#	Copyright (c) 2020 AT&T Intellectual Property.
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

# Build the user doc in ../library and if there are changes publish it into
# the scraper (../../../docs) directory.

sdir="../../../docs"	# the scraper dir

if [[ ! -d $sdir ]]
then
	echo "cannot find scraper directory: $sdir"
	exit 1
fi

set -e
cd ../library
make -B user.rst

new_m5=$( md5sum user.rst | sed 's/ .*//' )
old_md5=$( md5sum $sdir/user-guide.rst | sed 's/ .*//' )
	set -x
if [[ $new_m5 != $old_m5 ]]
then
	echo "publishing user-guide.rst"
	cp user.rst $sdir/user-guide.rst
ls -al $sdir/user-guide.rst
fi
	set +x
