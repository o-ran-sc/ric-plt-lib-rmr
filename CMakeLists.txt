
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

project( rmr LANGUAGES C )
cmake_minimum_required( VERSION 3.5 )

set( major_version "1" )		# should be automatically populated from git tag later, but until CI process sets a tag we use this
set( minor_version "0" )
set( patch_level "30" )

set( install_root "${CMAKE_INSTALL_PREFIX}" )
set( install_lib "lib" )
set( install_inc "include/rmr" )
if( MAN_PREFIX )
	set( install_man ${MAN_PREFIX} )			# is there a cmake var for this -- can't find one
else()
	set( install_man "/usr/share/man" )			# this needs to be fixed so it's not hard coded
endif()

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include( GNUInstallDirs )

# nano/nng install using LIBDIR as established by the gnu include; it varies from system
# to system, and we don't trust that it is always set, so we default to lib.
#
if( NOT CMAKE_INSTALL_LIBDIR )
	set( CMAKE_INSTALL_LIBDIR "lib" )
endif()

# ---------------- extract some things from git ------------------------------

# commit id for the version string
execute_process( 
	COMMAND bash -c "git rev-parse --short HEAD|awk '{printf\"%s\", $0}'" 
	OUTPUT_VARIABLE git_id
)

# version information for library names and version string
execute_process( 
	COMMAND bash -c "git describe --tags --abbrev=0 HEAD 2>/dev/null | awk -v tag=0.0.4095 ' { tag=$1 } END{ print  tag suffix }'|sed 's/\\./;/g' "
	OUTPUT_VARIABLE mmp_version_str
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

# extra indicator to show that the build was based on modified file(s) and not the true commit
# (no hope of reproducing the exact library for debugging). Used only for the internal version 
# string.
execute_process( 
	COMMAND bash -c "git diff --shortstat|awk -v fmt=%s -v r=-rotten '{ s=r } END { printf( fmt, s ) }'" 
	OUTPUT_VARIABLE spoiled_str
)

# uncomment these lines once CI starts adding a tag on merge
#set( mmp_version ${mmp_version_str} )
#list( GET mmp_version 0 major_version )
#list( GET mmp_version 1 minor_version )
#list( GET mmp_version 2 patch_level )

message( "+++ building ${major_version}.${minor_version}.${patch_level}${spoiled_str}" )

# define constants used in the version string
add_definitions( 
	-DGIT_ID=${git_id} 
	-DMAJOR_VER=${major_version}
	-DMINOR_VER=${minor_version}
	-DPATCH_VER=${patch_level}${spoiled_str}
)


# ---------------- setup nano/nng things ---------------------------------------
if( NOT SKIP_EXTERNALS )
	set( need_ext 1 )				# we force dependences on these for right build order
    execute_process( COMMAND  git submodule update --init -- ext/nng
            WORKING_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}
    )

    execute_process( COMMAND  git submodule update --init -- ext/nanomsg
            WORKING_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if( NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ext/nng/CMakeLists.txt )
        message( FATAL_ERROR "cannot find nng in our git source as a submodule: Giving up" )	# this will abort which seems wrong, but tdam.
    endif()

	include( ExternalProject )
	ExternalProject_Add(
		ext_nng
		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/nng"
		CMAKE_ARGS "-DBUILD_SHARED_LIBS=1"
		CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}"
		BUILD_COMMAND "make"
		UPDATE_COMMAND ""
		TEST_COMMAND ""
		STEP_TARGETS build
	)
	ExternalProject_Add(
		nanomsg
		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/nanomsg"
		BUILD_COMMAND "make"
		UPDATE_COMMAND ""
		CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}"
		TEST_COMMAND ""
		STEP_TARGETS build
	)

	# it seems impossible to install everything that lands in {bin}/lib, so we need to
	# hard code (shudder) some things. Even worse, we have to make exceptions for
	# builds on apple (osx) since their naming convention wandered off the path.
	set( nng_major 1 )
	set( nng_minor 1.0 )
	set( nano_major 5 )
	set( nano_minor 1.0 )
	set( so ${CMAKE_SHARED_LIBRARY_SUFFIX} )	# cmake variables are impossibly long :(
	if( NOT APPLE ) 				# probably breaks in windows, but idc
		set( nng_so_suffix ${so} )
		set( nng_so_suffix_m ${so}.${nng_major} )
		set( nng_so_suffix_mm ${so}.${nng_major}.${nng_minor} )

		set( nano_so_suffix ${so} )
		set( nano_so_suffix_m ${so}.${nano_major} )
		set( nano_so_suffix_mm ${so}.${nano_major}.${nano_minor} )
	else()
		# of course apple puts versions before the suffix :(
		set( nng_so_suffix ${so} )									# so has a lead dot, so NOT needed
		set( nng_so_suffix_m ".${nng_major}${so}" )					# these need leading dots
		set( nng_so_suffix_mm ".${nng_major}.${nng_minor}${so}" )

		set( nano_so_suffix ${so} )
		set( nano_so_suffix_m ".${nano_major}${so}" )
		set( nano_so_suffix_mm ".${nano_major}.${nano_minor}${so}" )
	endif()

	message( "+++ installing nano/nng with: ${nano_major}.${nano_minor} | ${nng_major}.${nng_minor}" )
else()
	set( need_ext 0 )
endif()
unset( SKIP_EXTERNALS  CACHE )	# prevent it from being applied next build unless specifically set on comd line


# this gets us round a chicken/egg problem. include files don't exist until make is run
# but Cmake insists on having these exist when we add them to include directories to
# enable rmr code to find them after we build them.
#
execute_process( COMMAND "mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/nng" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}/include" )


# Compiler flags
#
set( CMAKE_POSITION_INDEPENDENT_CODE ON )
set( CMAKE_CXX_FLAGS "-g -Wall " )

# Include modules
add_subdirectory( src/rmr/common )
add_subdirectory( src/rmr/nanomsg )
add_subdirectory( src/rmr/nng )
add_subdirectory( doc )				# this will auto skip if {X}fm is not available


# shared and static libraries built from the same object files
# Nanomsg based library (librmr )
# library is built by pulling object files from nano and common subdirs
#
add_library( rmr_shared SHARED "$<TARGET_OBJECTS:nano_objects>;$<TARGET_OBJECTS:common_objects>" )
add_library( rmr_static STATIC "$<TARGET_OBJECTS:nano_objects>;$<TARGET_OBJECTS:common_objects>" )

# both libraries to be named with librmr prefix and given version numbers with sym links
set_target_properties( rmr_shared 
	PROPERTIES 
		OUTPUT_NAME "rmr" 
		SOVERSION ${major_version} 
		VERSION ${major_version}.${minor_version}.${patch_level} )

set_target_properties( rmr_static 
	PROPERTIES 
		OUTPUT_NAME "rmr" 
		SOVERSION ${major_version} 
		VERSION ${major_version}.${minor_version}.${patch_level} )


# NNG based library (librmr_nng )
# library is built by pulling objects from nng and common subdirs.
#
add_library( rmr_nng_shared SHARED "$<TARGET_OBJECTS:nng_objects>;$<TARGET_OBJECTS:common_objects>" )
add_library( rmr_nng_static STATIC "$<TARGET_OBJECTS:nng_objects>;$<TARGET_OBJECTS:common_objects>" )



# if externals need to be built, then we must force them to be built first by depending on them
if( need_ext )
	add_dependencies( rmr_static;rmr_shared nanomsg )
	add_dependencies( rmr_nng_shared;rmr_nng_static ext_nng )
endif()


# both libraries to be named with librmr_nng prefix and given version numbers with sym links
set_target_properties( rmr_nng_shared 
	PROPERTIES 
		OUTPUT_NAME "rmr_nng" 
		SOVERSION ${major_version} 
		VERSION ${major_version}.${minor_version}.${patch_level} )

set_target_properties( rmr_nng_static 
	PROPERTIES 
		OUTPUT_NAME "rmr_nng" 
		SOVERSION ${major_version} 
		VERSION ${major_version}.${minor_version}.${patch_level} )

#
if( APPLE  )
	message( "### apple hack: forcing hard coded library paths for nng/nano dynamic libraries" )
	target_link_libraries( rmr_shared ${CMAKE_CURRENT_BINARY_DIR}/lib/libnanomsg${nano_so_suffix} )
	target_link_libraries( rmr_nng_shared ${CMAKE_CURRENT_BINARY_DIR}/lib/libnng${nng_so_suffix} )
endif()

# Define directories where package should drop things when installed
# In CMake speak archive == *.a  library == *.so, so both are needed
# Headers from the common directory are forced to install by the local CM file in common. At
# the moment, there are no header files specific to either nano or nng, so to the public
# header directive is moot, but needed if some day there is one.
#
#install( TARGETS rmr_nng_shared;rmr_nng_static;rmr_shared;rmr_static;rmr_nng_shared_mm EXPORT LibraryConfig
install( TARGETS rmr_nng_shared;rmr_nng_static;rmr_shared;rmr_static EXPORT LibraryConfig
    ARCHIVE  DESTINATION ${install_lib}
    LIBRARY  DESTINATION ${install_lib}
    PUBLIC_HEADER DESTINATION ${install_inc}
)



# install any nano/nng libraries in to the deb as well, but ONLY if we created them.
# (sure would be nice if FILEs allowed for globbing; sadlyy it does not.
#
if( need_ext )
	message( "including nano and nng libraries in the deb" )
	install( FILES
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnanomsg${nano_so_suffix}
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnanomsg${nano_so_suffix_m}
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnanomsg${nano_so_suffix_mm}
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnng${nng_so_suffix}
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnng${nng_so_suffix_m}
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnng${nng_so_suffix_mm}

		DESTINATION ${install_lib}
	)
endif()


IF( EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake" )
	include( InstallRequiredSystemLibraries )

	set( CPACK_set_DESTDIR "on" )
	set( CPACK_PACKAGING_INSTALL_PREFIX "${install_root}" )
	set( CPACK_GENERATOR "DEB" )

	set( CPACK_PACKAGE_DESCRIPTION "Thin library for RIC xAPP messaging routed based on message type." )
	set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "RIC message routing library" )
	set( CPACK_PACKAGE_VENDOR "None" )
	set( CPACK_PACKAGE_CONTACT "None" )
	set( CPACK_PACKAGE_VERSION_MAJOR "${major_version}" )
	set( CPACK_PACKAGE_VERSION_MINOR "${minor_version}" )
	set( CPACK_PACKAGE_VERSION_PATCH "${patch_level}" )
	set( CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${major_version}.${minor_version}.${CPACK_PACKAGE_VERSION_PATCH}${spoiled_str}" )
	set( CPACK_SOURCE_PACKAGE_FILE_NAME "vric${CMAKE_PROJECT_NAME}_${major_version}.${minor_version}.${CPACK_PACKAGE_VERSION_PATCH}" )

	# we build and ship the libraries, so there is NO dependency
	#set( CPACK_DEBIAN_PACKAGE_DEPENDS "nanomsg ( >= 1.1.4 ), nng ( >= 1.1.1 )" )

	set( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )
	set( CPACK_DEBIAN_PACKAGE_SECTION "ric" )
	set( CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )

	# this seems ingnored if included
	#set( CPACK_COMPONENTS_ALL Libraries ApplicationData )

	INCLUDE( CPack )
ENDIF()
