#!/usr/bin/env ksh

#==================================================================================
#        Copyright (c) 2019 Nokia
#        Copyright (c) 2018-2019 AT&T Intellectual Property.
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
#	Mnemonic:	cov2xml.ksh
#	Abstract:	Process the coverage file(s) which are read from standard in
#				to generate a single set of XML output that Sonar can ingest.
#				Basic usage is:
#					cat *.gcov | cov2xml.ksh >file.xml
#
#				The XML generated is based on the Sonar doc:
#					https://docs.sonarqube.org/latest/analysis/generic-test/
#
#	Date:		30 April 2019
#	Author:		E. Scott Daniels
# -------------------------------------------------------------------------

awk '
	function start() {
		printf(  "<coverage version=\"1\">\n" )		# sonar is VERY picky about this
	}

	function end() {
		printf( "</coverage>\n" )
	}

	function start_file( name ) {
	  printf( "<file path=\"%s\">\n", name )
	}

	function end_file( ) {
		printf( "</file>\n" )
	}

	function good_line( num ) {
		if( num > 0 ) {
			printf( "<lineToCover lineNumber=\"%d\" covered=\"true\"/>\n", num )
		}
	}

	function bad_line( num ) {
		if( num > 0 ) {
			printf( "<lineToCover lineNumber=\"%d\" covered=\"false\"/>\n", num )
		}
	}

	BEGIN {
		start()
	}

	/Source:/ {
		gsub( "../src", "src", $0 )		# sonar doesnt view test dir as root, so strip
		n = split( $0, a, ":" )
		if( fip ) {
			end_file()
		}
		fip = 1
		start_file( a[n] )
		next;
	}

	/-:/ { next }					# not executable

	/#####:/ {						# never executed
		bad_line( $2 + 0 )
		next
	}

	{
		good_line( $2 + 0 )
	}

	END {
		if( fip ) {
			end_file();
		}

		end()
	}
' $1

exit $/

