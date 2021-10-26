// vim: ts=4 sw=4 noet :
/*
==================================================================================
	Copyright 2021 Samsung Electronics All Rights Reserved.

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
	Mnemonic:	rmr_debug_si.c
	Abstract:	This is the compile point for the debug apis for si
        version of the rmr library

				API functions in this file will provide debug information
        of rmr

				Future:  the API functions can be added with necessity met
        for the rmr usage.

	Author:		Chung, Seung Wook
	Date:		12 October 2021
*/


/*
	rmr_get_rx_debug_count function will reset debug information of rmr rx queue
  both drop count and enqueue count of type uint64_t to zero.

  The vctx pointer is the pointer returned by the rmr_init function.

	On success function will return 0 otherwise it is an error.
  On error, errno will have failure reason, EINVAL.
*/
extern int rmr_reset_rx_debug_count(void *vctx) {
  uta_ctx_t *ctx;
  if ((ctx = (uta_ctx_t *)vctx) == NULL) {
    errno = EINVAL;
    return EINVAL;
  }
  ctx->acc_dcount = 0;
  ctx->acc_ecount = 0;
  return 0;
}

/*
	rmr_get_rx_debug_info function fills debug information of rmr rx status using
  rmr_rx_debug_t structure type. Debug information for RX status in rmr provides number
  of messages successfully queued to rmr and number of messages dropped for debug usage.

  The vctx pointer is the pointer returned by the rmr_init function. rx_debug is a pointer
  to a structure to receive rmr rx status information.

	On success function will return 0 otherwise it is an error.
  On error, errno will have failure reason, EINVAL.

	CAUTION:
		acc_dcount and acc_ecount will count in uint64_t range and will wrap-around when
    counted more. Two variables are counter thus will only increase.
*/
extern int rmr_get_rx_debug_info(void *vctx, rmr_rx_debug_t *rx_debug) {
  uta_ctx_t *ctx;
  if ((ctx = (uta_ctx_t *)vctx) == NULL) {
    errno = EINVAL;
    return EINVAL;
  }
  rx_debug->drop = ctx->acc_dcount;
  rx_debug->enqueue = ctx->acc_ecount;
  return 0;
}
