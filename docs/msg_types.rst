.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
RIC Message Types
============================================================================================


Overview
--------

RMR routes messages based on a combination of message type
and subscription ID. These types are defined in the
RIC_msg_types.h header file included with the RMR development
package.


Non-Routable Types
------------------

These types are not routable and a message passed to an RMR
function with any of these types set will not be sent.

   .. list-table::
     :widths: 30,70
     :header-rows: 0
     :class: borderless

     * - **RIC_UNDEFINED**
       -
         Mesage type is unset or undefined. Newly allocated messages
         have the type value set to this constant.




RMR Reserved types
------------------

These message types are reserved for RMR communications (e.g.
with Route Manager).

   .. list-table::
     :widths: 30,70
     :header-rows: 0
     :class: borderless

     * - **RMRRM_TABLE_DATA**
       -
         Table data from route manger. Route manager sends all route
         mse, etc.) with this type.

     * - **RMRRM_REQ_TABLE**
       -
         Request for table update. RMR will send a message with this a
         table update from route manger.

     * - **RMRRM_TABLE_STATE**
       -
         This message type conveys the state of the route table to the
         end of a table is noticed.




System Support Mesage Types
---------------------------

These message types are used for systems level communications
such as health checks, alarms and probes.

   .. list-table::
     :widths: 30,70
     :header-rows: 0
     :class: borderless

     * - **RIC_HEALTH_CHECK_REQ**
       -
         When received the application is expected to return a
         response to the current "health" of the application.

     * - **RIC_HEALTH_CHECK_RESP**
       -
         Health responses are sent with a message of this type.

     * - **RIC_ALARM**
       -
         Alarm messages with this type are routed to the alarm
         collection process.

     * - **RIC_ALARM_QUERY**
       -
         Unknown meaning

     * - **RIC_METRICS**
       -
         This message type causes the message to be routed to the xAPP
         responsible redistributing metrics.

     * - **RIC_SCTP_CONNECTION_FAILURE**
       -
         |

     * - **RIC_SCTP_CLEAR_ALL**
       -
         |

     * - **E2_TERM_INIT**
       -
         |

     * - **E2_TERM_KEEP_ALIVE_REQ**
       -
         |

     * - **E2_TERM_KEEP_ALIVE_RESP**
       -
         |

     * - **RAN_CONNECTED**
       -
         |

     * - **RAN_RESTARTED**
       -
         |

     * - **RAN_RECONFIGURED**
       -
         |

     * - **RIC_ENB_LOAD_INFORMATION**
       -
         |

     * - **RIC_ERROR_INDICATION**
       -
         |

     * - **RIC_SN_STATUS_TRANSFER**
       -
         |

     * - **RIC_UE_CONTEXT_RELEASE**
       -
         |

     * - **RIC_X2_SETUP_REQ**
       -
         |

     * - **RIC_X2_SETUP_RESP**
       -
         |

     * - **RIC_X2_SETUP_FAILURE**
       -
         |

     * - **RIC_X2_RESET**
       -
         |

     * - **RIC_X2_RESET_RESP**
       -
         |

     * - **RIC_ENB_CONF_UPDATE**
       -
         |

     * - **RIC_ENB_CONF_UPDATE_ACK**
       -
         |

     * - **RIC_ENB_CONF_UPDATE_FAILURE**
       -
         |

     * - **RIC_RES_STATUS_REQ**
       -
         |

     * - **RIC_RES_STATUS_RESP**
       -
         |

     * - **RIC_RES_STATUS_FAILURE**
       -
         |

     * - **RIC_RESOURCE_STATUS_UPDATE**
       -
         |

     * - **RIC_SGNB_ADDITION_REQ**
       -
         |

     * - **RIC_SGNB_ADDITION_ACK**
       -
         |

     * - **RIC_SGNB_ADDITION_REJECT**
       -
         |

     * - **RIC_SGNB_RECONF_COMPLETE**
       -
         |

     * - **RIC_SGNB_MOD_REQUEST**
       -
         |

     * - **RIC_SGNB_MOD_REQUEST_ACK**
       -
         |

     * - **RIC_SGNB_MOD_REQUEST_REJ**
       -
         |

     * - **RIC_SGNB_MOD_REQUIRED**
       -
         |

     * - **RIC_SGNB_MOD_CONFIRM**
       -
         |

     * - **RIC_SGNB_MOD_REFUSE**
       -
         |

     * - **RIC_SGNB_RELEASE_REQUEST**
       -
         |

     * - **RIC_SGNB_RELEASE_REQUEST_ACK**
       -
         |

     * - **RIC_SGNB_RELEASE_REQUIRED**
       -
         |

     * - **RIC_SGNB_RELEASE_CONFIRM**
       -
         |

     * - **RIC_RRC_TRANSFER**
       -
         |

     * - **RIC_ENDC_X2_SETUP_REQ**
       -
         |

     * - **RIC_ENDC_X2_SETUP_RESP**
       -
         |

     * - **RIC_ENDC_X2_SETUP_FAILURE**
       -
         |

     * - **RIC_ENDC_CONF_UPDATE**
       -
         |

     * - **RIC_ENDC_CONF_UPDATE_ACK**
       -
         |

     * - **RIC_ENDC_CONF_UPDATE_FAILURE**
       -
         |

     * - **RIC_SECONDARY_RAT_DATA_USAGE_REPORT**
       -
         |

     * - **RIC_GNB_STATUS_INDICATION**
       -
         |

     * - **RIC_E2_SETUP_REQ**
       -
         |

     * - **RIC_E2_SETUP_RESP**
       -
         |

     * - **RIC_E2_SETUP_FAILURE**
       -
         |

     * - **RIC_E2_RESET_REQ**
       -
         |

     * - **RIC_E2_RESET_RESP**
       -
         |

     * - **RIC_E2_RAN_ERROR_INDICATION**
       -
         |

     * - **RIC_E2_RIC_ERROR_INDICATION**
       -
         |

     * - **RAN_E2_RESET_REQ**
       -
         |

     * - **RAN_E2_RESET_RESP**
       -
         |

     * - **RIC_SUB_REQ**
       -
         |

     * - **RIC_SUB_RESP**
       -
         |

     * - **RIC_SUB_FAILURE**
       -
         |

     * - **RIC_SUB_DEL_REQ**
       -
         |

     * - **RIC_SUB_DEL_RESP**
       -
         |

     * - **RIC_SUB_DEL_FAILURE**
       -
         |

     * - **RIC_SERVICE_UPDATE**
       -
         |

     * - **RIC_SERVICE_UPDATE_ACK**
       -
         |

     * - **RIC_SERVICE_UPDATE_FAILURE**
       -
         |

     * - **RIC_CONTROL_REQ**
       -
         |

     * - **RIC_CONTROL_ACK**
       -
         |

     * - **RIC_CONTROL_FAILURE**
       -
         |

     * - **RIC_INDICATION**
       -
         |

     * - **RIC_SERVICE_QUERY**
       -
         |

     * - **DC_ADM_INT_CONTROL**
       -
         |

     * - **DC_ADM_INT_CONTROL_ACK**
       -
         |

     * - **DC_ADM_GET_POLICY**
       -
         |

     * - **DC_ADM_GET_POLICY_ACK**
       -
         |

     * - **A1_POLICY_REQ**
       -
         |

     * - **A1_POLICY_RESP**
       -
         |

     * - **A1_POLICY_QUERY**
       -
         |

     * - **TS_UE_LIST**
       -
         |

     * - **TS_QOE_PRED_REQ**
       -
         |

     * - **TS_QOE_PREDICTION**
       -
         |

     * - **MC_REPORT**
       -
         |

     * - **DCAPTERM_RTPM_RMR_MSGTYPE**
       -
         |

     * - **DCAPTERM_GEO_RMR_MSGTYPE**
       -
         |

     * - **RIC_X2_SETUP**
       -
         deprecated

     * - **RIC_X2_RESPONSE**
       -
         deprecated

     * - **RIC_X2_RESOURCE_STATUS_REQUEST**
       -
         deprecated

     * - **RIC_X2_RESOURCE_STATUS_RESPONSE**
       -
         deprecated

     * - **RIC_X2_LOAD_INFORMATION**
       -
         deprecated

     * - **RIC_E2_TERMINATION_HC_REQUEST**
       -
         deprecated

     * - **RIC_E2_TERMINATION_HC_RESPONSE**
       -
         deprecated

     * - **RIC_E2_MANAGER_HC_REQUEST**
       -
         deprecated

     * - **RIC_E2_MANAGER_HC_RESPONSE**
       -
         deprecated

     * - **RIC_CONTROL_XAPP_CONFIG_REQUEST**
       -
         |

     * - **RIC_CONTROL_XAPP_CONFIG_RESPONSE**
       -
         |


