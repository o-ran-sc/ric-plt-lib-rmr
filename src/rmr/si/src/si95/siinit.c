// vim: noet sw=4 ts=4:
/*
==================================================================================
    Copyright (c) 2020 Nokia
    Copyright (c) 2020 AT&T Intellectual Property.

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
**************************************************************************
*  Mnemonic: SIinitialise
*  Abstract: Initialisation and other context management functions.
*          
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
*  Mod:		17 FEB 2002 - To convert to a globally managed gpointer 
*			09 Mar 2007 - To allow for ipv6 (added SIinitialise() to 
*				replace SIinit())
**************************************************************************
*/
#include  "sisetup.h"     //  get the setup stuff 

/*
	Initialise the SI environment. Specifically:
		allocate the global info block (context)

	Returns a pointer to the block or nil on failure.
	On failure errno should indicate the problem.
*/
extern struct ginfo_blk* SIinitialise( int opts )
{
	struct ginfo_blk *gptr = NULL;	//  pointer at gen info blk 
	int 	status = SI_OK;			//  status of internal processing 
	struct	tp_blk *tpptr;         	//  pointer at tp stuff 
	struct	sigaction sact;			//  signal action block 
	int	i;							//  loop index 
	int	signals = SI_DEF_SIGS;		//  signals to be set in SIsetsig 

	errno = ENOMEM;

	if( (gptr = SInew( GI_BLK )) != NULL ) { 		//  make our context
		gptr->rbuf = (char *) malloc( MAX_RBUF );   //  get rcv buffer
		gptr->rbuflen = MAX_RBUF;
		gptr->tp_map = (struct tp_blk **) malloc( sizeof( struct tp_blk *) * MAX_FDS );
		if( gptr->tp_map == NULL ) {
			fprintf( stderr, "SIinit: unable to initialise tp_map: no memory\n" );
			free( gptr );
			return NULL;
		}
		memset( gptr->tp_map, 0, sizeof( struct tp_blk *) * MAX_FDS );

		gptr->sierr = SI_ERR_TPORT;
	
		gptr->cbtab = (struct callback_blk *) malloc(
			(sizeof( struct callback_blk ) * MAX_CBS ) );
		if( gptr->cbtab != NULL ) {
			for( i = 0; i < MAX_CBS; i++ ) {     //  initialize callback table 
				gptr->cbtab[i].cbdata = NULL;    //  no data and no functions 
				gptr->cbtab[i].cbrtn = NULL;
			}
		} else {                 //  if call back table allocation failed - error off 
			SIshutdown( gptr );  //  clean up any open fds 
			free( gptr );
			gptr = NULL;       //  dont allow them to continue 
		}

		gptr->sierr = SI_OK;
	}                     //  end if gen infor block allocated successfully 

	
	memset( &sact, 0, sizeof( sact ) );
	sact.sa_handler = SIG_IGN;
	sigaction( SIGPIPE, &sact, NULL );		// ignore pipe signals as for some bloody reason linux sets this off if write to closed socket

	return gptr;        	//  all's well that ends well 
} 

/*
	This will set all of the tcp oriented flags in mask (SI_TF_* constants).
*/
extern void SIset_tflags( struct ginfo_blk* gp, int mask )  {
	if( gp != NULL ) {
		gp->tcp_flags |= mask;
	}
}

/*
	This will clear all tcp oriented flags set in mask.
*/
extern void SIclr_tflags( struct ginfo_blk* gp, int mask )  {
	if( gp != NULL ) {
		gp->tcp_flags &= ~mask;
	}
}

/*
	Dump stats to stderr.

	NOTE:  the receive stats are the number of times that wait popped for
		a file descriptor and NOT the actual number of RMR messages which
		were contained.  Thus it is VERY likely that the receive count
		reported will not match the number of actual messages sent. These
		counts should be used only to track activity on a socket.
*/
extern void SItp_stats( void *vgp ) {
	struct ginfo_blk* gp;
	struct tp_blk* tp;

	if( (gp = (struct ginfo_blk *) vgp) != NULL ) {
		for( tp = gp->tplist; tp != NULL; tp = tp->next ) {
			rmr_vlog( RMR_VL_DEBUG, "si95: tp: fd=%d sent=%lld rcvd=%lld qc=%lld\n", tp->fd, tp->sent, tp->rcvd, tp->qcount );
		}
	}
}
