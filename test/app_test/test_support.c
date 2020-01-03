// :vim ts=4 sw=4 noet:
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
	Mnemonic:	test_support.c
	Abstract:	Support functions for app test programmes.

					sum() compute a simple checksum over a buffer
					split() split an ascii buffer at the first |
					generate_payload() build a header and data buffer generate
						a random data buffer with a header containing
						the checksum, and lengths.
					validate_msg() given a message of <hdr><data> validate
						that the message is the correct length and chk sum.

	Date:		28 October 2019
	Author:		E. Scott Daniels
*/

#include <ctype.h>

#ifndef _supp_tools_c
#define _supp_tools_c

#ifndef DEBUG
#define DEBUG 0
#endif

// defaults if the test app doesn't set these
#ifndef HDRSIZE
#define HDR_SIZE 64							// the size of the header we place into the message
#endif 

#ifndef MSG_SIZE
#define MSG_SIZE 1024						// the size of the message that will be sent (hdr+payload)
#endif

#ifndef DATA_SIZE
#define DATA_SIZE (HDR_SIZE-HDR_SIZE)		// the actual 'data' length returned in the ack msg
#endif

void spew( char* buf, int len ) {
	int i;
	char wbuf[1024];				// slower, but buffer so that mult writers to the tty don't jumble (too much)
	char bbuf[10];

	wbuf[0] = 0;
	for( i = 0; i < len; i++ ) {
		if( i % 16 == 0 ) {
			fprintf( stderr, "%s\n", wbuf );
			wbuf[0] = 0;
		}
		sprintf( bbuf, "%02x ", (unsigned char) *(buf+i) );
		strcat( wbuf, bbuf );
	}

	fprintf( stderr, "%s\n", wbuf );
}

/*
	Parse n bytes and generate a very simplistic checksum. 
	Returns the checksum.
*/
static int sum( char* bytes, int len ) {
	int sum = 0;
	int	i = 0;

	while( i < len ) {
		sum += *(bytes++) + i;
		i++;
	}

	return sum % 255;
}

/*
	Split the message at the first sep and return a pointer to the first
	character after.
*/
static char* split( char* str, char sep ) {
	char*	s;

	s = strchr( str, sep );

	if( s ) {
		return s+1;
	}

	fprintf( stderr, "<RCVR> no pipe in message: (%s)\n", str );
	return NULL;
}

/*
	Generate all buffers that will be combined into a single message
	payload. This is a random set of bytes into payload and then populate
	the header with the payload length and chksum. The header generated is
	placed into a fixed length buffer and consists of three values:
		checksum result of payload
		header size (the fixed size, though info is a string)
		payload size

	The message can be validated by computing the checksum of the
	message after the HDR_SIZE, and comparing that with the value
	passed.  The values in the header are all ASCII so they can
	easily be printed on receipt as a debugging step if needed.

	hs_over and ds_over allow the caller to override the header and data
	size constants if set to a positive number

	The return value is the size of the message payload needed to
	hold both the header and the payload.
*/
int generate_payload( char* hdr, char* data, int hs_over, int ds_over ) {
	int i;
	long r;
	int sv;
	int	hsize;
	int dsize;

	hsize = hs_over <= 0 ? HDR_SIZE : hs_over;		// set values from overrides or defaults
	dsize = ds_over <= 0 ? DATA_SIZE : ds_over;

	memset( hdr, 0, sizeof( char ) * hsize );
	for( i = 0; i < dsize; i++ ) {
		r = random();
		data[i] = r & 0xff;
	}

	sv = sum( data, dsize ); 
	snprintf( hdr, sizeof( char ) * hsize, "%d %d %d |", sv, hsize, dsize );

	return hsize + dsize;
}

/*
	Generate a header for a given payload buffer. THe header size and data size
	override values default to the HDR_SIZE and DATA_SIZE constants if a value
	is <= 0; The data is checksummed and  the header buffer is populated
	(see generate_payload() for detailed description.

	The return is the number of bytes required for the full payload of both
	header and data.
*/
int generate_header( char* hdr, char* data, int hs_over, int ds_over ) {
	int sv;
	int	hsize;
	int dsize;
	int	wrote;

	hsize = hs_over <= 0 ? HDR_SIZE : hs_over;		// set values from overrides or defaults
	dsize = ds_over <= 0 ? DATA_SIZE : ds_over;

	memset( hdr, 0, sizeof( char ) * hsize );
	sv = sum( data, dsize ); 
	wrote = snprintf( hdr, sizeof( char ) * hsize, "%d %d %d |", sv, hsize, dsize );
	if( wrote >= hsize ) {
		fprintf( stderr, "<ERR>  header overflow\n" );
		hdr[hsize-1] = 0;
	}

	return hsize + dsize;
}

/*
	Convenience function to push a header/data into an RMR mbuf. It is assumed that the
	mbuf is large enough; no checks are made. If either length is <= 0, then the 
	default (constant value) is used.
*/
void fill_payload( rmr_mbuf_t* msg, char* hdr, int hdr_len, char *data, int data_len ) {
	char*	payload;

	if( msg == NULL ) {
		fprintf( stderr, "<TEST> fill payload: msg passed in was nil\n" );
		return;
	}

	hdr_len = hdr_len <= 0 ? HDR_SIZE : hdr_len;		// set values from input; else use defaults
	data_len = data_len <= 0 ? DATA_SIZE : data_len;

	if( hdr_len <= 0 || data_len <= 0 ) {
		fprintf( stderr, "<TEST> fill payload: header or date len invalid: hdr=%d data=%d\n", hdr_len, data_len );
		return;
	}

	payload = msg->payload;

	memcpy( payload, hdr, hdr_len > 0 ? hdr_len : HDR_SIZE );
	payload += hdr_len;
	memcpy( payload, data, data_len > 0 ? data_len : DATA_SIZE );
}

/*
	This function accepts an RMR message payload assumed to be header and user data,  in the 
	format:	<hdr><data> where <hdr> consists of a string with three space separated values:
		checksum
		hdr size
		data size

	Validation is:
		ensure that the rmr_len matches the header size and data size which are parsed from
		the message.  If sizes match, a checksum is performed on the data and the result
		compared to the checksum value in the header.
*/
int validate_msg( char* buf, int rmr_len ) {
	char*	tok;
	char*	tok_mark = NULL;
	char*	search_start;
	char*	ohdr = NULL;	// original header as we trash the header parsing it
	int		ex_sv;			// expected checksum value (pulled from received header)
	int		sv;				// computed checksum value
	int		ex_mlen = 0;	// expected msg_len (computed from the header, compared with rmr_len)
	int		hdr_len;		// length of header to skip
	int		data_len;		// length of data received
	int		i;
	int		good = 0;

	if( buf == 0 && rmr_len <= 0 ) {
		fprintf( stderr, "<TEST> validate msg: nil buffer or no len: %p len=%d\n", buf, rmr_len );
		return 0;
	}

	for( i = 0; i < HDR_SIZE; i++ ) {		// for what we consider the dfault header, we must see ALL ascci and a trail zero before end
		if( *(buf+i) == 0 ) {
			good = 1;
			break;
		}

		if( ! isprint( *buf ) ) {  // we expect the header to be a zero terminated string
			fprintf( stderr, "<TEST> validate msg: header is not completely ASCII i=%d\n", i );
			spew( buf, rmr_len > 64 ? 64 : rmr_len );
			return 0;
		}
	}

	if( ! good ) {
		fprintf( stderr, "<TEST> validate msg: didn't find an acceptable header (not nil terminated)\n" );
		return 0;
	}

	ohdr = strdup( buf );		// must copy so we can trash

	search_start = ohdr;
	for( i = 0; i < 3; i++ ) {
		tok = strtok_r( search_start, " ", &tok_mark );
		search_start = NULL;
		if( tok ) {
			switch( i ) {
				case 0:
					ex_sv = atoi( tok );
					break;

				case 1:
					hdr_len = atoi( tok );		// capture header length 	
					ex_mlen = hdr_len;			// start to compute total msg len
		
					break;

				case 2:
					data_len = atoi( tok );		// add data length 	
					ex_mlen += data_len;
					break;
			}
		}
	}

	if( ex_mlen != rmr_len ) {
		fprintf( stderr, "[FAIL] received message length did not match hdr+data lengths, rmr_len=%d data follows:\n", rmr_len );
		if( ! isprint( *ohdr ) ) {
			fprintf( stderr, "<TEST> validate msg: header isn't printable\n" );
			spew( ohdr, rmr_len > 64 ? 64 : rmr_len );
		} else {
			fprintf( stderr, "<TEST> validate msg: header: (%s)\n", ohdr );
			fprintf( stderr, "<TEST> validate msg: computed length: %d (expected)\n", ex_mlen );
		}
		free( ohdr );
		return 0;
	}

	if( DEBUG ) {
		fprintf( stderr, "[OK]  message lengths are good\n" );
	}
		
	tok = buf + hdr_len;
	//fprintf( stderr, ">>>> computing chksum starting at %d for %d bytes\n", hdr_len, data_len );
	sv = sum( tok, data_len );			// compute checksum of data portion

	if( sv != ex_sv ) {
		fprintf( stderr, "<TEST> validate msg:  data checksum mismatch, got %d, expected %d. header: %s\n", sv, ex_sv, ohdr );
		free( ohdr );
		return 0;
	}
	
	free( ohdr );
	return 1;	
}


#endif
