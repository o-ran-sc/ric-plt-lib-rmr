// :vi sw=4 ts=4 noet:
/*
==================================================================================
	Copyright (c) 2019-2020 Nokia
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
	Mnemonic:	rmr_logging.h
	Abstract:	All needed header stuff for RMR's common logger.
		
	Author:		E. Scott Daniels
	Date:		27 January 2020
*/

#ifndef _logger_h
#define _logger_h


// situation constants
#define LOG_ERROR	"ERR"
#define LOG_WARN	"WARN"
#define LOG_INFO	"INFO"
#define LOG_CRIT	"CRIT"
#define LOG_DEBUG	"DBUG"

/*
	vlevel constants; ordered such that if( vlevel >= write_level  ) is true when we have a 
	verbose level setting condusive for writing. 
*/
#define RMR_VL_DEBUG 	5
#define RMR_VL_INFO		4
#define RMR_VL_WARN		3
#define RMR_VL_ERR		2
#define RMR_VL_CRIT		1
#define RMR_VL_OFF		0


// ----- prototypes ------------------------------
extern void rmr_vlog( int write_level, char* fmt, ... );
extern void rmr_vlog_force( int write_level, char* fmt, ... );
extern int rmr_vlog_init();
extern void rmr_set_vlevel( int new_level );



#endif

