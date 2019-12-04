// : vi ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2019 Nokia
	    Copyright (c) 2018-2019 AT&T Intellectual Property.

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
	Mnemonic:	test_gen_rt.c
	Abstract:	This provides the means to generate a route table to disk.
	Author:		E. Scott Daniels
	Date:		6 January 2019
*/

#ifndef _test_gen_rt_c
#define _test_gen_rt_c


/*
	Generate a simple route table (for all but direct route table testing).
	This gets tricky inasmuch as we generate two in one; first a whole table 
	and then two update tables. The first is a table with a bad counter in the
	last record to test that we don't load that table and error. The second
	is a good update. The same applies to the meid map; first has a bad counter
	and some bad records to drive coverage testing. The end should leave a good
	meid map in the table.
*/
static void gen_rt( uta_ctx_t* ctx ) {
	int		fd;
	char* 	rt_stuff;		// strings for the route table

	fd = open( "utesting.rt", O_WRONLY | O_CREAT, 0600 );
	if( fd < 0 ) {
		fprintf( stderr, "<BUGGERED> unable to open file for testing route table gen\n" );
		return;
	}

	rt_stuff =
		"newrt|end\n"								// end of table check before start of table found
		"# comment to drive full comment test\n"
		"\n"										// handle blank lines
		"   \n"										// handle blank lines
	    "mse|4|10|localhost:4561\n"					// entry before start message
	    "rte|4|localhost:4561\n"					// entry before start message
		"newrt|start\n"								// false start to drive detection
		"xxx|badentry to drive default case"
		"newrt|start\n"
	    "rte|0|localhost:4560,localhost:4562\n"					// these are legitimate entries for our testing
	    "rte|1|localhost:4562;localhost:4561,localhost:4569\n"
	    "rte|2|localhost:4562| 10\n"								// new subid at end
	    "mse|4|10|localhost:4561\n"									// new msg/subid specifier rec
	    "mse|4|localhost:4561\n"									// new mse entry with less than needed fields
		"   rte|   5   |localhost:4563    #garbage comment\n"		// tests white space cleanup
	    "rte|6|localhost:4562\n"
		"newrt|end\n";

	setenv( "RMR_SEED_RT", "utesting.rt", 1 );
	write( fd, rt_stuff, strlen( rt_stuff ) );				// write in the whole table

	rt_stuff = 												// add an meid map which will fail
		"meid_map | start\n"
		"mme_ar | e2t-1 | one two three four\n"
		"mme_del | one two\n"
		"mme_del \n"										// short entries drive various checks for coverage
		"mme_ar \n"											
		"mme_ar | e2t-0 \n"									
		"meid_map | end | 5\n";								// this will fail as the short recs don't "count"
	write( fd, rt_stuff, strlen( rt_stuff ) );

	rt_stuff =
		"updatert|start\n"									// this is an update to the table
	    "mse|4|99|fooapp:9999,barapp:9999;logger:9999\n"	// update just one entry
		"updatert|end | 3\n";								// bad count; this update should be rejected
	write( fd, rt_stuff, strlen( rt_stuff ) );


	rt_stuff =
		"updatert|start\n"									// this is an update to the table
	    "mse|4|10|fooapp:4561,barapp:4561;logger:9999\n"	// update just one entry
		"mse | 99 | -1 | %meid\n"							// type 99 will route based on meid and not mtype
		"del|2|-1\n"										// delete an entry; not there so no action
		"del|2|10\n"										// delete an entry
		"updatert|end | 4\n";								// end table; updates have a count as last field
	write( fd, rt_stuff, strlen( rt_stuff ) );

	rt_stuff = 												// this leaves an meid map in place too
		"meid_map | start\n"
		"mme_ar | localhost:4567 | meid1 meid2 meid3 meid4\n"
		"mme_ar | localhost:4067 | meid11 meid12\n"
		"meid_map | end | 2\n";
	write( fd, rt_stuff, strlen( rt_stuff ) );

	rt_stuff = 													// verify that we can del entries in the current table
		"meid_map | start\n"
		"mme_del | meid11 meid12 meid13\n"		// includes a non-existant meid
		"meid_map | end | 1\n";
	write( fd, rt_stuff, strlen( rt_stuff ) );
	
	close( fd );
	read_static_rt( ctx, 1 );								// force in verbose mode to see stats on tty if failure
	unlink( "utesting.rt" );
}



/*
	Generate a custom route table file using the buffer passed in.
*/
static void gen_custom_rt( uta_ctx_t* ctx, char* buf ) {
	int		fd;
	char* 	rt_stuff;		// strings for the route table

	fd = open( "utesting.rt", O_WRONLY | O_CREAT, 0600 );
	if( fd < 0 ) {
		fprintf( stderr, "<BUGGERED> unable to open file for testing route table gen\n" );
		return;
	}
	setenv( "RMR_SEED_RT", "utesting.rt", 1 );

	write( fd, rt_stuff, strlen( buf ) );

	close( fd );
	read_static_rt( ctx, 1 );								// force in verbose mode to see stats on tty if failure
	unlink( "utesting.rt" );
}


#endif
