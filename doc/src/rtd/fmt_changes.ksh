# vim: ts=4 noet sw=4:
#==================================================================================
#	Copyright (c) 2019 Nokia
#	Copyright (c) 2018-2019 AT&T Intellectual Property.
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

#	Mnemonic:	fmt_changes.ksh
#	Abstract:	This script looks for CHANGES*.txt files at the top level
#				and builds one {X}fm source file from which the release notes
#				RTD file is created.

cat <<endKat

.dv GEN_TITLE 1
.dv doc_title RMR Release Notes
.im setup.im
.dh 1 u=off

&h1(RMR Release Notes)
The following is a list of release highlights for the RMR library.
At one point in time the RMR repo also housed a wrapper library with a
separate version and release cycle.
This resulted in &ital(leap frogging) versions for each package; the
RMR core library was assigned odd major numbers (e.g. 3.1.0).
When the wrapper code was moved to a different repo the need to leap frog
versions ceased, and beginning with version 4.0.0, the RMR versions should
no longer skip.
&space

endKat

for x in ../../../CHANGES*.txt
do
	sed 's/^/!/' $x | awk '
		print_raw && /^!$/ {
			printf( "&space\n\n" );
			next
		}

		{ gsub ( "!", "", $1 ) }

		$1 + 0 >= 2019 {
			print_raw = 1
			printf( "&h2(%s)\n", $0 )
			next
		}

		print_raw { print }
		' ###
done
