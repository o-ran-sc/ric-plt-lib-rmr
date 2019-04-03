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




/* Header  file defining  message types 
   for various RMR messages 


    ------------------
    WORK IN PROGRESS
    ------------------

*/

#define RIC_UNDEFINED                       -1

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
