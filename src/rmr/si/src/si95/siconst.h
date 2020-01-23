// vim: noet sw=4 ts=4:
/*
==================================================================================
	Copyright (c) 2020 Nokia
	Copyright (c) 2018-2020 AT&T Intellectual Property.

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
*****************************************************************************
*
*  Mnemonic: siconst.h
*  Abstract: Private constants for SI functions.
*
*  Date:	 26 March 1995
*  Author:   E. Scott Daniels
*
*
*****************************************************************************
*/

#ifndef _siconst_h
#define _siconst_h

#ifndef DEBUG
#define DEBUG 0
#endif 


//#define EOS   '\000'		 //  end of string marker 

								//  general info block flags 
#define GIF_SHUTDOWN	0x01   //  shutdown in progress 
#define GIF_NODELAY		0x02   //  set no delay flag on t_opens 

								//  transmission provider block flags 
#define TPF_LISTENFD	0x01   //  set on tp blk that is fd for tcp listens 
#define TPF_SESSION		0x02   //  session established on this fd 
#define TPF_UNBIND		0x04   //  SIterm should unbind the fd 
#define TPF_DRAIN		0x08   //  session is being drained 
#define TPF_DELETE		0x10	//  block is ready for deletion -- when safe 

#define MAX_CBS			8	 //  number of supported callbacks in table 
#define MAX_RBUF		8192   //  max size of receive buffer 
#define MAX_FDS			2048	// max number of file descriptors

#define TP_BLK	0			 //  block types for rsnew 
#define GI_BLK	1			 //  global information block 
#define IOQ_BLK	2			 //  input output block 

#define MAGICNUM	219		//  magic number for validation of things 

#define AC_TODOT  0			 //  convert address to dotted decimal string 
#define AC_TOADDR 1			 //  address conversion from dotted dec 
#define AC_TOADDR6 2		//  ipv6 address conversion from string to addr struct 
#define AC_TOADDR6_4BIND 3	//  ipv6 address conversion from string to addr struct suitible for bind 

#define NO_EVENT 0			//  look returns 0 if no event on a fd 

#endif
