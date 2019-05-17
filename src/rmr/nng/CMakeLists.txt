
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

# just to be clear, this generates object files, not a library as the CM command implies
add_library( nng_objects OBJECT src/rmr_nng.c )

if( need_ext ) 
	add_dependencies( nng_objects ext_nng )		# if we are building, ensure built first
endif()

target_include_directories (nng_objects PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../rmr/include>
	PRIVATE src)

#	$<INSTALL_INTERFACE:include>
