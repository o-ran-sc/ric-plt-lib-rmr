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
.im setup.im
&h1(RMR Release Notes)
The following is a list of release highlights for the core RMR library and 
wrappers which are housed in the same repository.
These are extracted directly from the CHANGES_*.txt files at the repo root; 
please refer to those files for a completely up to date listing of
API changes (as generated documents may lag).  
&space

The RMR repo houses two distinct release entities: the core RMR package
and the python wrapper package.
To avoid naming conflicts (tags mostly) The core package uses odd 
major version numbers (e.g. 3.2.1) and the wrapper package uses even
major version numbers. 
The release notes are split into two sections; please be sure to 
scroll to the section that is appropriate.

endKat

for x in ../../../CHANGES*.txt
do
	case $x in 
		*CORE*) printf "&h2(Core RMR Changes)\n" ;;
		*)	printf "&h2(Wrapper Changes)\n" ;;
	esac

	sed 's/^/!/' $x | awk ' 
		print_raw && /^!$/ { 
			printf( "&space\n\n" ); 
			next 
		} 
	
		{ gsub ( "!", "", $1 ) }
	
		$1 + 0 >= 2019 {
			print_raw = 1
			printf( "&h3(%s)\n", $0 )
			next 
		}
	
		print_raw { print } 
		' ###
done
