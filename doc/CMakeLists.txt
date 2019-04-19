
#
#==================================================================================
#	Copyright (c) 2019 Nokia 
#	Copyright (c) 2018-2019 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#==================================================================================
#


# builds the man pages for the deb file (generates troff from {X}fm).
# also builds postscript files, but leaves them in the current build dir.
# but, ONLY if build_doc variable is true
#

# look for tfm to build the man pages with. if not found, then we pull
# and build in the current build environment setting tfm/pfm commands to
# point at the correct spot.  If the user has {X}fm installed, we just
# use their install.
#
if( BUILD_DOC )
	find_program( tfm NAMES tfm )
	find_program( pfm NAMES pfm )

	if( "${tfm}" MATCHES "tfm-NOTFOUND" )		# user doesn't have installed; set where we expect them
		set( tfm ${CMAKE_CURRENT_BINARY_DIR}/xfm/.build/src/tfm/tfm )
		set( pfm "${CMAKE_CURRENT_BINARY_DIR}/xfm/.build/src/pfm/pfm" )

		if( NOT EXISTS ${tfm} )					# not yet built here, pull and build
			# pull and build {X}fm tools needed to generate manpages
			execute_process( 
				COMMAND "bash" "-c" "git clone https://github.com/ScottDaniels/xfm.git && cd xfm && mkdir .build && cd .build && cmake .. && make"
				WORKING_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}
			)
			message( "+++ xfm  pulled and built" )
		else()
			message( "+++ found xfm in the build environment" )
		endif()
	
	endif()

	# base filenames (with .xfm are input)
	set( man_names
		rmr.7
		rmr_bytes2meid.3
		rmr_bytes2xact.3
		rmr_bytes2payload.3
		rmr_free_msg.3
		rmr_payload_size.3
		rmr_rts_msg.3
		rmr_wh_close.3
		rmr_alloc_msg.3
		rmr_get_rcvfd.3
		rmr_rcv_msg.3
		rmr_send_msg.3
		rmr_wh_open.3
		rmr_call.3
		rmr_init.3
		rmr_ready.3
		rmr_str2meid.3
		rmr_str2xact.3
		rmr_support.3
		rmr_torcv_msg.3
		rmr_wh_send_msg.3
		rmr_get_trace.3
		rmr_init_trace.3
		rmr_set_trace.3
		rmr_tralloc_msg.3
		rmr_get_trlen.3
		rmr_get_src.3
	)

	# empty list of roff/troff input files we generated
	set( man3_files )
	set( man7_files )

	# mk is so much easier than this -- grumble
	# for each source, build a specific command that runs tfm to generate the
	# troff output. Sed is needed to remove the leading blank that tfm likes
	# to insert even if indention is 0.
	#
	foreach( nm IN LISTS man_names )
	  set( out ${CMAKE_BINARY_DIR}/${nm} )
	  set( in ${CMAKE_SOURCE_DIR}/doc/src/man/${nm}.xfm )

	  add_custom_command( 
			OUTPUT ${out}
			DEPENDS ${in}
			COMMAND bash -c "export LIB=${CMAKE_SOURCE_DIR}/doc/src; ${tfm} ${in} stdout | sed 's/^ //' >${out}; ${pfm} ${in} ${out}.ps"
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			COMMENT "Building manpage ${out}"
			VERBATIM
		)
		if( ${out} MATCHES ".*\.3" )
			list( APPEND man3_files ${out} )
		else()
			list( APPEND man7_files ${out} )
		endif()
	endforeach()

	# we must force these to install
	# find all of the man pages in build and add them to the deb
	#
	install( FILES ${man3_files} DESTINATION ${install_man}/man3/ )
	install( FILES ${man7_files} DESTINATION ${install_man}/man7/ )

	add_custom_target( man_pages ALL DEPENDS ${man3_files};${man7_files} )

else()
	message( "+++ not building doc, set -DBULID_DOC on cmake commandline to enable" )
endif()
unset( BUILD_DOC  CACHE )    # prevent it from being applied next build unless specifically set on comd line
