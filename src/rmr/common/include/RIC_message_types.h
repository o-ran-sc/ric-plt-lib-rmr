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
	Header  file defining  message types for various RMR messages
*/

#define RIC_UNDEFINED				-1

/* 
---------------------------------------------------------
	RMR Reserved types
		All message types 0 - 99 are reserved for RMM.
---------------------------------------------------------
*/
		
#define RMRRM_TABLE_DATA			20		// table data from route manger
#define	RMRRM_REQ_TABLE				21		// request for table update to route mangager
#define RMRRM_TABLE_STATE			22		// state of table to route mgr


// --- please keep additions in numerical order ------

#define RIC_SCTP_CONNECTION_FAILURE 1080
#define RIC_SCTP_CLEAR_ALL			1090

#define	E2_TERM_INIT				1100
#define E2_TERM_KEEP_ALIVE_REQ		1101
#define E2_TERM_KEEP_ALIVE_RESP		1102

#define	RAN_CONNECTED				1200
#define	RAN_RESTARTED				1210
#define	RAN_RECONFIGURED			1220


#define RIC_ENB_LOAD_INFORMATION	10020

#define RIC_ERROR_INDICATION		10030

#define	RIC_SN_STATUS_TRANSFER		10040

#define RIC_UE_CONTEXT_RELEASE		10050

#define RIC_X2_SETUP_REQ			10060
#define RIC_X2_SETUP_RESP			10061
#define RIC_X2_SETUP_FAILURE		10062
#define RIC_X2_RESET				10070
#define	RIC_X2_RESET_RESP			10071

#define RIC_ENB_CONF_UPDATE			10080
#define RIC_ENB_CONF_UPDATE_ACK		10081
#define RIC_ENB_CONF_UPDATE_FAILURE	10082

#define RIC_RES_STATUS_REQ			10090
#define RIC_RES_STATUS_RESP			10091
#define RIC_RES_STATUS_FAILURE		10092

#define RIC_RESOURCE_STATUS_UPDATE	10100

#define RIC_SGNB_ADDITION_REQ		10270
#define RIC_SGNB_ADDITION_ACK		10271
#define RIC_SGNB_ADDITION_REJECT	10272
#define RIC_SGNB_RECONF_COMPLETE	10280

#define	RIC_SGNB_MOD_REQUEST		10290
#define	RIC_SGNB_MOD_REQUEST_ACK	10291
#define	RIC_SGNB_MOD_REQUEST_REJ	10292

#define	RIC_SGNB_MOD_REQUIRED		10300
#define	RIC_SGNB_MOD_CONFIRM		10301
#define	RIC_SGNB_MOD_REFUSE			10302

#define	RIC_SGNB_RELEASE_REQUEST	10310
#define	RIC_SGNB_RELEASE_REQUEST_ACK 10311

#define	RIC_SGNB_RELEASE_REQUIRED	10320
#define	RIC_SGNB_RELEASE_CONFIRM	10321

#define RIC_RRC_TRANSFER			10350

#define RIC_ENDC_X2_SETUP_REQ		10360
#define RIC_ENDC_X2_SETUP_RESP		10361
#define RIC_ENDC_X2_SETUP_FAILURE	10362

#define RIC_ENDC_CONF_UPDATE		10370
#define RIC_ENDC_CONF_UPDATE_ACK	10371
#define RIC_ENDC_CONF_UPDATE_FAILURE	10372

#define	RIC_SECONDARY_RAT_DATA_USAGE_REPORT 10380

#define RIC_GNB_STATUS_INDICATION	10450

#define RIC_SUB_REQ					12010
#define RIC_SUB_RESP				12011
#define RIC_SUB_FAILURE				12012

#define RIC_SUB_DEL_REQ				12020
#define RIC_SUB_DEL_RESP			12021
#define RIC_SUB_DEL_FAILURE			12022

#define RIC_SERVICE_UPDATE			12030
#define RIC_SERVICE_UPDATE_ACK		12031
#define RIC_SERVICE_UPDATE_FAILURE	12032

#define RIC_CONTROL_REQ				12040
#define RIC_CONTROL_ACK				12041
#define RIC_CONTROL_FAILURE			12042

#define RIC_INDICATION				12050

#define RIC_SERVICE_QUERY			12060

#define DC_ADM_INT_CONTROL			20000
#define DC_ADM_INT_CONTROL_ACK		20001
#define DC_ADM_GET_POLICY			20002
#define DC_ADM_GET_POLICY_ACK		20003

#define A1_POLICY_REQ      			20010
#define A1_POLICY_RESP    			20011
#define A1_POLICY_QUERY				20012




// ---- these are old (release 0) definitions and should not be used ------

/* E2 Related messages  should be in the range
   10000 to 99999
*/

#define RIC_X2_SETUP                     10000
#define RIC_X2_RESPONSE                  10001
#define RIC_X2_RESOURCE_STATUS_REQUEST   10002
#define RIC_X2_RESOURCE_STATUS_RESPONSE  10003
#define RIC_X2_LOAD_INFORMATION          10004
#define RIC_E2_TERMINATION_HC_REQUEST    10005
#define RIC_E2_TERMINATION_HC_RESPONSE   10006
#define RIC_E2_MANAGER_HC_REQUEST        10007
#define RIC_E2_MANAGER_HC_RESPONSE       10008


/* A1 Related messages should be in the range
   100000 to 999999
*/
#define RIC_CONTROL_XAPP_CONFIG_REQUEST  100000
#define RIC_CONTROL_XAPP_CONFIG_RESPONSE 100001
