      /tmp/x.rst RMR LIBRARY FUNCTIONS ===================== NAME ---- 
      rmr_init SYNOPSIS -------- :: #include <rmr/rmr.h> void* 
      rmr_init( char* proto_port, int norm_msg_size, int flags ); 
      DESCRIPTION ----------- The function prepares the environment 
      for sending and receiving messages. It does so by establishing a 
      worker thread (pthread) which subscribes to a route table 
      generator which provides the necessary routing information for 
      the RMR library to send messages. *Port* is used to listen for 
      connection requests from other RMR based applications. The 
      *norm_msg_size* parameter is used to allocate receive buffers 
      and should be set to what the user application expects to be a 
      size which will hold the vast majority of expected messages. 
      When computing the size, the application should consider the 
      usual payload size **and** the maximum trace data size that will 
      be used. This value is also used as the default message size 
      when allocating message buffers (when a zero size is given to 
      rmr_alloc_msg(); see the rmr_alloc_msg() manual page). Messages 
      arriving which are longer than the given normal size will cause 
      RMR to allocate a new buffer which is large enough for the 
      arriving message. Starting with version 3.8.0 RMR no longer 
      places a maximum buffer size for received messages. The 
      underlying system memory manager might impose such a limit and 
      the attempt to allocate a buffer larger than that limit will 
      likely result in an application abort. Other than the potential 
      performance impact from extra memory allocation and release, 
      there is no penality to the user programme for specifyning a 
      normal buffer size which is usually smaller than received 
      buffers. Similarly, the only penality to the application for 
      over specifying the normal buffer size might be a larger memory 
      footprint. *Flags* allows for selection of some RMR options at 
      the time of initialisation. These are set by ORing constants 
      from the RMR header file. Currently the following flags are 
      supported: .. list-table:: :widths: auto :header-rows: 0 :class: 
      borderless * - **RMRFL_NONE** - No flags are set. | * - 
      **RMRFL_NOTHREAD** - The route table collector thread is not to 
      be started. This should only be used by the route table 
      generator application if it is based on RMR. | * - 
      **RMRFL_MTCALL** - Enable multi-threaded call support. | * - 
      **RMRFL_NOLOCK** - Some underlying transport providers (e.g. 
      SI95) enable locking to be turned off if the user application is 
      single threaded, or otherwise can guarantee that RMR functions 
      will not be invoked concurrently from different threads. Turning 
      off locking can help make message receipt more efficient. If 
      this flag is set when the underlying transport does not support 
      disabling locks, it will be ignored. Multi-threaded Calling 
      ---------------------- The support for an application to issue a 
      *blocking call* by the function was limited such that only user 
      applications which were operating in a single thread could 
      safely use the function. Further, timeouts were message count 
      based and not time unit based. Multi-threaded call support adds 
      the ability for a user application with multiple threads to 
      invoke a blocking call function with the guarantee that the 
      correct response message is delivered to the thread. The 
      additional support is implemented with the *rmr_mt_call()* and 
      *rmr_mt_rcv()* function calls. Multi-threaded call support 
      requires the user application to specifically enable it when RMR 
      is initialised. This is necessary because a second, dedicated, 
      receiver thread must be started, and requires all messages to be 
      examined and queued by this thread. The additional overhead is 
      minimal, queuing information is all in the RMR message header, 
      but as an additional process is necessary the user application 
      must "opt in" to this approach. ENVIRONMENT ----------- As a 
      part of the initialisation process reads environment variables 
      to configure itself. The following variables are used if found. 
      .. list-table:: :widths: auto :header-rows: 0 :class: borderless 
      * - **RMR_ASYNC_CONN** - Allows the async connection mode to be 
      turned off (by setting the value to 0). When set to 1, or 
      missing from the environment, RMR will invoke the connection 
      interface in the transport mechanism using the non-blocking 
      (async) mode. This will likely result in many "soft failures" 
      (retry) until the connection is established, but allows the 
      application to continue unimpeded should the connection be slow 
      to set up. | * - **RMR_BIND_IF** - This provides the interface 
      that RMR will bind listen ports to, allowing for a single 
      interface to be used rather than listening across all 
      interfaces. This should be the IP address assigned to the 
      interface that RMR should listen on, and if not defined RMR will 
      listen on all interfaces. | * - **RMR_CTL_PORT** - This variable 
      defines the port that RMR should open for communications with 
      Route Manager, and other RMR control applications. If not 
      defined, the port 4561 is assumed. Previously, the (route table 
      generator service port) was used to define this port. However, a 
      future version of Route Manager will require RMR to connect and 
      request tables, thus that variable is now used to supply the 
      Route Manager's well-known address and port. To maintain 
      backwards compatibility with the older Route Manager versions, 
      the presence of this variable in the environment will shift 
      RMR's behaviour with respect to the default value used when is 
      **not** defined. When is **defined:** RMR assumes that Route 
      Manager requires RMR to connect and request table updates is 
      made, and the default well-known address for Route manager is 
      used (routemgr:4561). When is **undefined:** RMR assumes that 
      Route Manager will connect and push table updates, thus the 
      default listen port (4561) is used. To avoid any possible 
      misinterpretation and/or incorrect assumptions on the part of 
      RMR, it is recommended that both the and be defined. In the case 
      where both variables are defined, RMR will behave exactly as is 
      communicated with the variable's values. | * - **RMR_RTG_SVC** - 
      The value of this variable depends on the Route Manager in use. 
      When the Route Manager is expecting to connect to an xAPP and 
      push route tables, this variable must indicate the which RMR 
      should use to listen for these connections. When the Route 
      Manager is expecting RMR to connect and request a table update 
      during initialisation, the variable should be the of the Route 
      Manager process. The variable (added with the support of sending 
      table update requests to Route manager), controls the behaviour 
      if this variable is not set. See the description of that 
      variable for details. | * - **RMR_HR_LOG** - By default RMR 
      writes messages to standard error (incorrectly referred to as 
      log messages) in human readable format. If this environment 
      variable is set to 0, the format of standard error messages 
      might be written in some format not easily read by humans. If 
      missing, a value of 1 is assumed. | * - **RMR_LOG_VLEVEL** - 
      This is a numeric value which corresponds to the verbosity level 
      used to limit messages written to standard error. The lower the 
      number the less chatty RMR functions are during execution. The 
      following is the current relationship between the value set on 
      this variable and the messages written: .. list-table:: :widths: 
      auto :header-rows: 0 :class: borderless * - **0** - Off; no 
      messages of any sort are written. | * - **1** - Only critical 
      messages are written (default if this variable does not exist) | 
      * - **2** - Errors and all messages written with a lower value. 
      | * - **3** - Warnings and all messages written with a lower 
      value. | * - **4** - Informational and all messages written with 
      a lower value. | * - **5** - Debugging mode -- all messages 
      written, however this requires RMR to have been compiled with 
      debugging support enabled. | * - **RMR_RTG_ISRAW** - 
      **Deprecated.** Should be set to 1 if the route table generator 
      is sending "plain" messages (not using RMR to send messages), 0 
      if the RTG is using RMR to send. The default is 1 as we don't 
      expect the RTG to use RMR. This variable is only recognised when 
      using the NNG transport library as it is not possible to support 
      NNG "raw" communications with other transport libraries. It is 
      also necessary to match the value of this variable with the 
      capabilities of the Route Manager; at some point in the future 
      RMR will assume that all Route Manager messages will arrive via 
      an RMR connection and will ignore this variable. | * - 
      **RMR_SEED_RT** - This is used to supply a static route table 
      which can be used for debugging, testing, or if no route table 
      generator process is being used to supply the route table. If 
      not defined, no static table is used and RMR will not report 
      *ready* until a table is received. The static route table may 
      contain both the route table (between newrt start and end 
      records), and the MEID map (between meid_map start and end 
      records). | * - **RMR_SRC_ID** - This is either the name or IP 
      address which is placed into outbound messages as the message 
      source. This will used when an RMR based application uses the 
      rmr_rts_msg() function to return a response to the sender. If 
      not supplied RMR will use the hostname which in some container 
      environments might not be routable. The value of this variable 
      is also used for Route Manager messages which are sent via an 
      RMR connection. | * - **RMR_VCTL_FILE** - This supplies the name 
      of a verbosity control file. The core RMR functions do not 
      produce messages unless there is a critical failure. However, 
      the route table collection thread, not a part of the main 
      message processing component, can write additional messages to 
      standard error. If this variable is set, RMR will extract the 
      verbosity level for these messages (0 is silent) from the first 
      line of the file. Changes to the file are detected and thus the 
      level can be changed dynamically, however RMR will only suss out 
      this variable during initialisation, so it is impossible to 
      enable verbosity after startup. | * - **RMR_WARNINGS** - If set 
      to 1, RMR will write some warnings which are non-performance 
      impacting. If the variable is not defined, or set to 0, RMR will 
      not write these additional warnings. RETURN VALUE ------------ 
      The function returns a void pointer (a context if you will) that 
      is passed as the first parameter to nearly all other RMR 
      functions. If is unable to properly initialise the environment, 
      NULL is returned and errno is set to an appropriate value. 
      ERRORS ------ The following error values are specifically set by 
      this RMR function. In some cases the error message of a system 
      call is propagated up, and thus this list might be incomplete. 
      .. list-table:: :widths: auto :header-rows: 0 :class: borderless 
      * - **ENOMEM** - Unable to allocate memory. EXAMPLE ------- :: 
      void* uh; rmr_mbuf* buf = NULL; uh = rmr_init( "43086", 4096, 0 
      ); buf = rmr_rcv_msg( uh, buf ); SEE ALSO -------- 
      rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
      rmr_get_rcvfd(3), rmr_mt_call(3), rmr_mt_rcv(3), 
      rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
      rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), 
      rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
      rmr_ring_free(3) 
