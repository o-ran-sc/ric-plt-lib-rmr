
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

# simulate the route table manager
project( rtm_sim LANGUAGES C )
cmake_minimum_required( VERSION 3.5 )

set( major_version "1" )
set( minor_version "0" )
set( patch_level "1" )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )
set( CMAKE_CXX_FLAGS "-g -Wall -I /usr/local/include" )

find_library( nanomsg libnanomsg.a )

find_program( rpm NAMES rpmbuild )					# rpm package gen requires this to be installed

if( "${rpm}" MATCHES "rpm-NOTFOUND" ) 				# cannot build rpm
	set( pkg_list "DEB" )
	message( "+++ `make package` will generate only deb package; cannot find support to generate rpm packages" )
else()
	set( pkg_list "DEB;RPM" )
	message( "+++ `make package` will generate both deb and rpm packages" )
endif()

add_executable( rtm_sim rtm_sim.c )
target_link_libraries( rtm_sim nanomsg )

install( 
	TARGETS rtm_sim
	DESTINATION /usr/local/bin
)

IF( EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake" )
	include( InstallRequiredSystemLibraries )

	set( CPACK_set_DESTDIR "on" )
	set( CPACK_PACKAGING_INSTALL_PREFIX "${install_root}" )
	set( CPACK_GENERATOR ${pkg_list} )

	set( CPACK_PACKAGE_DESCRIPTION "Simplistic route table manager simulator." )
	set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "RT manager simulation" )
	set( CPACK_PACKAGE_VENDOR "None" )
	set( CPACK_PACKAGE_CONTACT "None" )
	set( CPACK_PACKAGE_VERSION_MAJOR "${major_version}" )
	set( CPACK_PACKAGE_VERSION_MINOR "${minor_version}" )
	set( CPACK_PACKAGE_VERSION_PATCH "${patch_level}" )
	set( CPACK_PACKAGE_FILE_NAME "rtm_sim-${major_version}.${minor_version}.${patch_level}-${CMAKE_SYSTEM_PROCESSOR}" )

	# we build and ship the libraries, so there is NO dependency
	set( CPACK_DEBIAN_PACKAGE_DEPENDS "libnanomsg0 (>=0.4)" )

	set( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )
	set( CPACK_DEBIAN_PACKAGE_SECTION "test" )
	set( CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )
	set( CPACK_RPM_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )

	INCLUDE( CPack )
ENDIF()
