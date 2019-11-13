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


# This provides a quick sanity check on the message types in
# the header file. The check is only to ensure that there are
# no duplicate constant names or values for any #define in the file.
#
# By default RIC_message_types.h is parsed, but it will accept the 
# filename as the first positional parameter on the command line should
# it be necessary.
#
# The script exits with a 0 return code if all is good, 1 to indicate errors.
#
# CAUTION:  this breaks if any define is more than a simple key/value
#	pair in the header file.

awk '
	/#define/ { 
		vcount[$NF]++; 
		ncount[$2]++;
		next
	} 

	END { 
		vgood = 0
		ngood = 0
		bad = 0

		for( x in vcount ) { 
			if( vcount[x] != 1 ) { 
				printf( "duplicate value? %s\n", x ); 
				bad++ 
			} else { 
				vgood++ 
			} 
		} 

		for( x in ncount ) { 
			if( ncount[x] != 1 ) { 
				printf( "duplicate name? %s\n", x ); 
				bad++ 
			} else { 
				ngood++ 
			} 
		} 

		printf( "good values=%d good names=%d  bad things=%d\n", vgood, ngood, bad ) 

		exit( !! bad )
	}' ${1:-RIC_message_types.h}
