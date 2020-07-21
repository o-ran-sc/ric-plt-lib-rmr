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

# build separate man pages as .rst files and publish to the scrapable directory if
# there are changes. We build the .rst and if it is different than what is in the
# scarper directory we will copy it over there.  We inject a title so that the
# RTD scripts won't object.

sdir="${RTD_SCRAPER_DIR:-../../../docs}"		# the scraper dir (allow for testing/alternate)

if [[ ! -d $sdir ]]
then
	echo "cannot find scraper directory: $sdir"
	exit 1
fi

mkdir -p stuff
ls -al ../man/*.xfm|sed 's!^.*man/!!' | while read x
do
	if true
	then
		out=stuff/${x%.*}.rst
		target=${sdir}/${out##*/}

		INPUT_FILE=${x%%.*} GEN_TITLE=1 LIB=".." OUTPUT_TYPE=rst tfm ../man/$x stdout 2>/dev/null | sed 's/^ //;  s/ *$//' >$out
ls -al $out
		new_m5=$( md5sum $out | sed 's/ .*//' )
		if [[ ! -f $target || $new_m5 != $( md5sum $target | sed 's/ .*//' ) ]]
		then
			echo "publishing to: ${target##*/}"
			cp $out $target
		fi


		#rm $out
	fi
done

#rmdir stuff
