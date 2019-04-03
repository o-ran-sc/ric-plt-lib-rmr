
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


add_library( common_objects OBJECT 
	src/symtab.c
	src/mbuf_api.c
)

target_include_directories (common_objects PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	PRIVATE src)

# we have to force headers to install
install( FILES 
	include/rmr.h
	include/rmr_symtab.h
	include/RIC_message_types.h
	DESTINATION ${install_root}/${install_inc}
)	
