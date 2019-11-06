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

# format the changes file at the top level into xfm input
cat <<endKat
.im setup.im
&h1(RMR Release Notes)
The following is a list of release highlights for the core RMR library.
These are extracted directly from the CHANGES file at the repo root; 
please refer to that file for a completely up to date listing of
API changes.  
&space

endKat

sed 's/^/!/' ../../../CH*|awk ' 

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
	'
