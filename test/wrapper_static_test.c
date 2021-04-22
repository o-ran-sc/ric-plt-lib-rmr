// : vi ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2021 Nokia
	    Copyright (c) 2021 AT&T Intellectual Property.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
==================================================================================
*/


/*
	Mnemonic:	wrapper_static_test.c
	Abstract:	Unit test for the wrapper module in common.

	Author:		E. Scott Daniels
	Date:		19 April 2020
*/

#include <wrapper.c>

/*
	Called by the test driver (main). Returns the number of errors found.
*/
static int wrapper_test( ) {
	int errors = 0;
	char*	b;
	int		len;

	b = rmr_get_consts();		// function that builds constant json string for python-like things

	if( fail_if_equal( strlen( b ), 0, "wrapper buffer had nothing" ) ) {
		return 1;						// can't do any further checking
	}

	errors += fail_if_true( *b != '{', "first character in buffer not valid json" );
	len = strlen( b ) - 1;
	errors += fail_if_true( *(b+len) != '}', "last character in buffer not valid json" );
	free( b );

	b = build_sval( "foobar", "value", 1 );
	errors += fail_if_equal( strlen( b ), 0, "build svalue with sep returned nil buffer" );
	errors += fail_not_equal( strcmp( b, "\"foobar\": \"value\"," ), 0, "svalue result not the expected string" );

	b = build_sval( "foobar", "value", 0 );
	errors += fail_if_equal( strlen( b ), 0, "build svalue without sep returned nil buffer" );
	errors += fail_not_equal( strcmp( b, "\"foobar\": \"value\"" ), 0, "svalue result without sep not the expected string" );


	// -------------------------------------------------------------------------------------------------

	return errors;
}
