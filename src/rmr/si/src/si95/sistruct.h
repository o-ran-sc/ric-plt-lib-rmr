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
***************************************************************************
*
* Mnemonic: sistruct.h
* Abstract: This file contains the structure definitions ncessary to support
*           the SI (Socket interface) routines.
* Date:     26 March 1995
* Author:   E. Scott Daniels
*
******************************************************************************
*/

struct ioq_blk              //  block to queue on session when i/o waiting 
{
	struct ioq_blk *next;     //  next block in the queue 
	char *data;               //  pointer to the data buffer 
	unsigned int dlen;        //  data length 
	void *addr;               //  address to send to (udp only) 
	int alen;		//  size of address struct (udp) 
 };

struct callback_blk         //  defines a callback routine 
{
	void *cbdata;            //  pointer to be passed to the call back routine 
	int ((*cbrtn)( ));       //  pointer to the callback routine 
};

struct tp_blk               //  transmission provider info block 
{
	struct tp_blk *next;      	//  chain pointer 
	struct tp_blk *prev;      	//  back pointer 
	int fd;                   	//  open file descriptor 
	int flags;                	//  flags about the session / fd 
	int type;                 	//  connection type SOCK_DGRAM/SOCK_STREAM 
	int family;					//  address family  AF_* constants from system headers
	struct sockaddr *addr; 		//  connection local address 
	struct sockaddr *paddr; 	//  session partner's address 
	int		palen;				//	length of the struct referenced by paddr (connect needs)
	struct ioq_blk *squeue;   	//  queue to send to partner when it wont block 
	struct ioq_blk *sqtail;   	//  last in queue to eliminate the need to search 

								// a few counters for stats
	long long qcount;			// number of messages that waited on the queue
	long long sent;				// send/receive counts
	long long rcvd;
};

struct ginfo_blk {				//  general info block  (context)
	unsigned int magicnum;		//  magic number that ids a valid block 
	struct tp_blk *tplist;		//  pointer at tp block list 
	fd_set readfds;				//  select/poll file des lists
	fd_set writefds;
	fd_set execpfds;
	char *rbuf;					//  read buffer 
	struct callback_blk *cbtab; //  pointer at the callback table 
	int fdcount;				//  largest fd to select on in siwait 
	int flags;					//  status flags 
	int	tcp_flags;				// connection/session flags (e.g. no delay)
	int rbuflen;				//  read buffer length 
	int	sierr;					// our internal error number (SI_ERR_* constants)
	struct tp_blk**	tp_map;		// direct fd -> tp block map
};
