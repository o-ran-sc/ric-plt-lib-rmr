#!/usr/bin/env ksh
# :vim ts=4 sw=4 noet:
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

#	Mnemonic:	ci_build.ksh
#	Abstract:	Script builds RMr, exececutes the unit and application based
#				tests, then generates packages. The packages are left in /tmp
#				unless -t target-dir is given on the command line. A YAML file
#				is created which can be used to find all of the packages which
#				were deposited in the target directory. In an environment which
#				is also capable of generating RPM ppackages, there should be
#				four packages: a run-time and development package for both
#				debian (.deb) and rh (rpm) based systems.
#
#				The intent of this script is for it to be executed as a part
#				of a docker image build process such that the resulting image
#				contains the RMr packages, and a YAML file which provides the
#				paths to the package(s) in the image.  Docker tools can then
#				be used to extract the packages and push them to some external
#				repository.
#
#				Assumptions:
#					We assume that this scirpt is executed at the 'root' of the
#				RMr repo (i.e. the directory which has a subdirectory ci).
#
#	Date:		14 June 2019
# --------------------------------------------------------------------------------

# stash a set of packages for a particular flavour ($1). Records what was kept
# in a yaml file for an external process to grab, and in a simple publication
# list for our internal publish script to gobble up.
#
function stash_pkgs {
	for pkg in deb rpm
	do
		ls .build/*.$pkg 2>/dev/null | while read f
		do
			cp $f $target_dir/${f##*/}
			echo "  - $target_dir/${f##*/}" >>$yaml_file
			echo "$target_dir/${f##*/}" >>$pub_file
		done

	done
}

# ---------------------------------------------------------------------------

target_dir="/tmp"
verbose=0

while [[ $1 == -* ]]
do
	case $1 in
		-t)	target_dir=$2; shift;;
		-v)	verbose=1;;

		*)	echo "$1 is not recognised"
			echo ""
			echo "usage: $0 [-t target-dir]"
			echo "   target dir defaults to /tmp and is where packages and lists are placed."
			exit 1
			;;
	esac

	shift
done

if [[ ! -d $target_dir ]]
then
	if ! -mkdir -p $target_dir
	then
		echo "[FAIL] cannot find or create target directory: $target_dir"
		exit 1
	fi
fi

if [[ ! -d ./ci ]]							# verify we are in the root of the RMr repo filesystem, abort if not
then
	echo "[FAIL] current working directory does not seem right; should be RMr repo root: $PWD"
	exit 1
fi

set -e										# fail unconditionally on first issue
yaml_file=$target_dir/build_packages.yml
pub_file=$target_dir/build_packages.txt		# just a list of files built for the publish script (json/yaml is overkill)
rm -f $yaml_file
rm -f $pub_file

mkdir -p .build
(
	cd .build
	rm -f *.deb *.rpm						# these shouldn't be there, but take no chances
	cmake .. -DBUILD_DOC=1 -DDEV_PKG=1		# set up dev config for unit test (requires dev stuff)
	make package
)
(
	cd test                 		# execute tests
	ksh unit_test.ksh				# unit tests first
	cd app_test
	ksh run_all.ksh					# application based tests if units pass
)

printf "---\nfiles:\n" >$yaml_file 	# initialise the yaml file

stash_pkgs development      # testing good, stash dev packages built above

(
	cd .build				# now build the run-time packages
	rm -f *.deb *.rpm		# reconfig should delete these, but take no chances
	cmake ..                # configure run-time build
	make package
)
stash_pkgs runtime			# move packages to target and record in yaml file

echo "..." >>$yaml_file		# terminate yaml file

set +e
if (( verbose ))
then
	echo "generated yaml file:"
	cat $yaml_file
	echo ""
	echo "------"
	ls -al $target_dir/*.deb $target_dir/*.rpm
fi

exit 0
