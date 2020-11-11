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
-----------------------------------------------------------------------------------
	Mnemonic: siproto.h
	Abstract: Prototypes for SI95

	Date:     24 October 2019
	Author:   E. Scott Daniels

-----------------------------------------------------------------------------------
*/

#ifndef _si_proto_h
#define _si_proto_h

extern void siabort_conn( int fd );		// use by applications discouraged

extern void *SInew( int type );
extern char *SIgetname( int sid );
extern void SIabort( struct ginfo_blk *gptr );
extern int SIaddress( void *src, void **dest, int type );
extern void SIbldpoll( struct ginfo_blk* gptr  );
extern struct tp_blk *SIconn_prep( struct ginfo_blk *gptr, int type, char *abuf, int family );
extern void SIcbreg( struct ginfo_blk *gptr, int type, int ((*fptr)()), void * dptr );
extern void SIcbstat( struct ginfo_blk *gptr, int status, int type );
extern int SIclose( struct ginfo_blk *gptr, int fd );
extern int SIconnect( struct ginfo_blk *gptr, char *abuf );
extern struct tp_blk *SIestablish( int type, char *abuf, int family );
extern int SIgenaddr( char *target, int proto, int family, int socktype, struct sockaddr **rap );
extern int SIgetaddr( struct ginfo_blk *gptr, char *buf );
extern struct tp_blk *SIlisten_prep( int type, char* abuf, int family );
extern int SIlistener( struct ginfo_blk *gptr, int type, char *abuf );
extern void SImap_fd( struct ginfo_blk *gptr, int fd, struct tp_blk* tpptr );
extern int SInewsession( struct ginfo_blk *gptr, struct tp_blk *tpptr );
extern int SIpoll( struct ginfo_blk *gptr, int msdelay );
extern int SIrcv( struct ginfo_blk *gptr, int sid, char *buf, int buflen, char *abuf, int delay );
extern void SIrm_tpb( struct ginfo_blk *gptr, struct tp_blk *tpptr );
extern void SIsend( struct ginfo_blk *gptr, struct tp_blk *tpptr );
extern int SIsendt( struct ginfo_blk *gptr, int fd, char *ubuf, int ulen );
extern void SIset_tflags( struct ginfo_blk* gp, int flags );
extern int SIshow_version( );
extern void SIshutdown( struct ginfo_blk *gptr );
extern void SItp_stats( void *vgp );
extern void SIterm( struct ginfo_blk* gptr, struct tp_blk *tpptr );
extern void SItrash( int type, void *bp );
extern int SIwait( struct ginfo_blk *gptr );
extern struct ginfo_blk* SIinitialise( int opts );

#endif
