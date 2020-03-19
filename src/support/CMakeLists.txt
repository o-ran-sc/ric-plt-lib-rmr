#
#==================================================================================
#	Copyright (c) 2020 Nokia
#	Copyright (c) 2020 AT&T Intellectual Property.
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

# build support tools and setup for their install
add_executable( health_ck health_ck.c )
add_dependencies( health_ck rmr_si_static )
target_link_libraries( health_ck rmr_si_static;pthread;m )

include_directories( ${CMAKE_SOURCE_DIR}/src/rmr/common/include )


# install only into the runtime package
if( NOT DEV_PKG )
	install(
    	TARGETS health_ck
    	DESTINATION bin
	)
endif()

