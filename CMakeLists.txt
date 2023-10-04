#
#==================================================================================
#	Copyright (c) 2019-2021 Nokia
#	Copyright (c) 2018-2021 AT&T Intellectual Property.
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

# This CMake definition supports several -D command line options:
#
#	-DDEBUG=n			Enable debugging level n
#	-DDEV_PKG=1			Development package configuration
#	-DBUILD_DOC=1		Man pages generated
#	-DBUILD_NNG=1		Enable building of NNG and the RMR NNG based libraries
#	-DIGNORE_LIBDIR=1	Installation of rmr libries is into /usr/local/lib and ignores
#						value in CMAKE_INSTALL_LIBDIR.
#						system preferred (typically /usr/local/lib64).
#	-DPRESERVE_PTYPE=1	Do not change the processor type when naming deb packages
#	-DPACK_EXTERNALS=1	Include external libraries used to build in the run-time package
#						(This makes some stand-alone unit testing of bindings possible, it
#						is not meant to be used for production package generation.)
#	-DGPROF=1			Enable profiling compile time flags
#	-DSKIP_EXTERNALS=1	Do not use NNG submodule when building; uee installed packages
#	-DMAN_PREFIX=<path>	Supply a path where man pages are installed (default: /usr/share/man)
#	-DOPT_LEVEL=n		Set a custom optimisation level.

#	See ci/build_all for an example of how to build and test

project( rmr LANGUAGES C )
cmake_minimum_required( VERSION 3.5 )

set( major_version "4" )		# should be automatically populated from git tag later, but until CI process sets a tag we use this
set( minor_version "9" )
set( patch_level "2" )

set( install_root "${CMAKE_INSTALL_PREFIX}" )
set( install_inc "include/rmr" )
if( MAN_PREFIX )
	set( install_man ${MAN_PREFIX} )			# is there a cmake var for this -- can't find one
else()
	set( install_man "/usr/share/man" )			# this needs to be fixed so it's not hard coded
endif()

# Must use GNUInstallDirs to install libraries into correct locations on all platforms.
include( GNUInstallDirs )

# We install using LIBDIR as established by the gnu include; it varies from system
# to system, and we don't trust that it is always set, so we default to lib if it is missing.
#
if( NOT CMAKE_INSTALL_LIBDIR )
	set( CMAKE_INSTALL_LIBDIR "lib" )
endif()

if( IGNORE_LIBDIR )					# if set, then force to lib otherwise use "system preference"
	set( install_lib "lib" )
else()
	set( install_lib "${CMAKE_INSTALL_LIBDIR}" )
endif()
unset(IGNORE_LIBDIR CACHE )					# we don't want this to persist
message( "+++ RMR library install target directory: ${install_lib}" )

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
message( "+++ mmp version from tag: '${mmp_version_str}'" )

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

if( DEBUG )					# if set, we'll set debugging on in the compile
	set( debugging ${DEBUG} )
	message( "+++ debugging is being set to ${DEBUG}" )
else()
	set( debugging 0 )
	message( "+++ debugging is set to off (use -DDEBUG=1 to enable)" )
endif()
unset( DEBUG CACHE )					# we don't want this to persist


# define constants used in the version string, debugging, etc.
add_definitions(
	-DGIT_ID=${git_id}
	-DMAJOR_VER=${major_version}
	-DMINOR_VER=${minor_version}
	-DPATCH_VER=${patch_level}
	-DDEBUG=${debugging}
)

# ---------------- suss out pkg gen tools so we don't fail generating packages that the system cannot support --------------

# deb packages use underbars, and package manager(s) seem to flip the *_64 processor type
# to the old (non-standard) amd64 string, so we do it here for consistency. Set -DPRESERVE_PTYPE=1
# to prevent the flip. RPM packages will always be given the system generated processor type string.
#
if( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" )
	if( NOT PRESERVE_PTYPE )
		set( deb_sys_name "amd64" )
	else()
		set( deb_sys_name ${CMAKE_SYSTEM_PROCESSOR} )
	endif()
else()
	set( deb_sys_name ${CMAKE_SYSTEM_PROCESSOR} )
endif()
unset( PRESERVE_PTYPE CACHE )					# we don't want this to persist

set( rpm_sys_name ${CMAKE_SYSTEM_PROCESSOR} )

if( DEV_PKG )
	set( deb_pkg_name "rmr-dev" )
	set( rpm_pkg_name "rmr-devel" )
else()
	set( deb_pkg_name "rmr" )
	set( rpm_pkg_name "rmr" )
endif()

set( pkg_label "rmr${spoiled_str}-${major_version}.${minor_version}.${patch_level}-${sys_name}" )
set( rpm_pkg_label "${rpm_pkg_name}${spoiled_str}-${major_version}.${minor_version}.${patch_level}-${rpm_sys_name}" )
set( deb_pkg_label "${deb_pkg_name}${spoiled_str}_${major_version}.${minor_version}.${patch_level}_${deb_sys_name}" )
message( "+++ pkg name: ${deb_pkg_label}.deb" )

#set( out_yml /tmp/build_output.yml )					# we will record package names (we record only untainted names)
find_program( rpm NAMES rpmbuild )						# rpm package gen requires this to be installed

set( gen_rpm 0 )
if( "${rpm}" MATCHES "rpm-NOTFOUND" )			       # cannot build rpm
	set( pkg_list "DEB" )
	message( "### make package will generate only deb package; cannot find support to generate rpm packages" )
else()
	message( "+++ pkg name: ${rpm_pkg_label}.rpm" )		# debugging if we think we can gen rpm too
	set( pkg_list "DEB;RPM" )
	set( gen_rpm 1 )
	message( "+++ make package will generate both deb and rpm packages" )
endif()

# ---------------- setup nano/nng things ---------------------------------------
if( NOT BUILD_NNG )
	set( PACK_EXTERNALS 0 )
else()
	if( NOT SKIP_EXTERNALS )
		set( need_ext 1 )				# we force dependences on these for right build order
		execute_process( COMMAND  git submodule update --init -- ext/nng
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

		# it seems impossible to install everything that lands in {bin}/lib, so we need to
		# hard code (shudder) some things. Even worse, we have to make exceptions for
		# builds on apple (osx) since their naming convention wandered off the path.
		set( nng_major 1 )
		set( nng_minor 1.0 )
		set( so ${CMAKE_SHARED_LIBRARY_SUFFIX} )	# cmake variables are impossibly long :(
		if( NOT APPLE )								# probably breaks in windows, but idc
			set( nng_so_suffix ${so} )
			set( nng_so_suffix_m ${so}.${nng_major} )
			set( nng_so_suffix_mm ${so}.${nng_major}.${nng_minor} )
		else()
			# of course apple puts versions before the suffix :(
			set( nng_so_suffix ${so} )									# so has a lead dot, so NOT needed
			set( nng_so_suffix_m ".${nng_major}${so}" )					# these need leading dots
			set( nng_so_suffix_mm ".${nng_major}.${nng_minor}${so}" )
		endif()

		message( "+++ building with nng: ${nng_major}.${nng_minor}" )
	else()
		if( PACK_EXTERNALS )
			# This makes some stand-alone unit testing possible for bindings and transport layer testing;
			# it is not meant for production packages.
			#
			unset( SKIP_EXTERNALS  CACHE )	# must remove so as not to trap user into a never ending failure
			unset( PACK_EXTERNALS  CACHE )
			message( FATAL_ERROR "ERROR: PACK_EXTERNALS can be set only if SKIP_EXTERNALS is unset (=0, or not supplied on command line)" )
		endif()
		set( need_ext 0 )
	endif()
endif()



# this gets us round a chicken/egg problem. include files don't exist until make is run
# but Cmake insists on having these exist when we add them to include directories to
# enable rmr code to find them after we build them.
#
execute_process( COMMAND "mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/nng" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}/include" )


# Compiler flags
#
set( CMAKE_POSITION_INDEPENDENT_CODE ON )
#set( CMAKE_C_FLAGS "-g -Wall " )
#set( CMAKE_C_FLAGS "-g " )
if( GPROF )					# if set, we'll set profiling flag on compiles
	message( "+++ profiling is on" )
	set( CMAKE_C_FLAGS "-pg " )
else()
	message( "+++ profiling is off (use -DGPROF=1 to enable" )
	set( CMAKE_C_FLAGS "-g " )
endif()
unset( GPROF CACHE )					# we don't want this to persist

if( DEFINED OPT_LEVEL )
	message( "+++ optimisation is forced to ${OPT_LEVEL}" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O${OPT_LEVEL} " )
else()
	message( "+++ optimisation will default (use -DOPT_LEVEL=n to force specific level, or disable)" )
endif()
unset( OPT_LEVEL  CACHE )			# no optimisation flage does NOT percist

message( "+++ compiler flags: ${CMAKE_C_FLAGS}" )


# Include modules
add_subdirectory( src/rmr/common )
if( BUILD_NNG )
	add_subdirectory( src/rmr/nng )
endif()
add_subdirectory( src/rmr/si )
add_subdirectory( doc )				# this will auto skip if {X}fm is not available


# shared and static libraries are built from the same object files.
# librmr_* is a combination of common and * specific rmr functions.
#

if( BUILD_NNG )
	add_library( rmr_nng_shared SHARED "$<TARGET_OBJECTS:nng_objects>;$<TARGET_OBJECTS:common_objects>" )
	set_target_properties( rmr_nng_shared
		PROPERTIES
		OUTPUT_NAME "rmr_nng"
		SOVERSION ${major_version}
		VERSION ${major_version}.${minor_version}.${patch_level} )

	# we only build/export the static archive (.a) if generating a dev package
	if( DEV_PKG )
		add_library( rmr_nng_static STATIC "$<TARGET_OBJECTS:nng_objects>;$<TARGET_OBJECTS:common_objects>" )
		set_target_properties( rmr_nng_static
			PROPERTIES
			OUTPUT_NAME "rmr_nng"
			SOVERSION ${major_version}
			VERSION ${major_version}.${minor_version}.${patch_level} )
	endif()
endif()

add_library( rmr_si_shared SHARED "$<TARGET_OBJECTS:rmr_si_objects>;$<TARGET_OBJECTS:common_objects>" )
set_target_properties( rmr_si_shared
	PROPERTIES
	OUTPUT_NAME "rmr_si"
	SOVERSION ${major_version}
	VERSION ${major_version}.${minor_version}.${patch_level} )

# even if not generating a development package we still need to generate the .a so that health check
# can link against it to avoid RPM install issues.
#
add_library( rmr_si_static STATIC "$<TARGET_OBJECTS:rmr_si_objects>;$<TARGET_OBJECTS:common_objects>" )
set_target_properties( rmr_si_static
	PROPERTIES
	OUTPUT_NAME "rmr_si"
	SOVERSION ${major_version}
	VERSION ${major_version}.${minor_version}.${patch_level} )

if( BUILD_NNG )
	# if externals need to be built, then we must force them to be built first by depending on them
	if( need_ext )
		if( DEV_PKG )
			add_dependencies( rmr_nng_shared;rmr_nng_static ext_nng )
		else()
			add_dependencies( rmr_nng_shared ext_nng )
		endif()
	endif()
endif()

# ------------- testing -------------------------------------------------------
enable_testing()
# cmake cannot set env vars, so we have to passed desired vars on the wrapper command
add_test(
		NAME drive_unit_tests
		COMMAND bash ../test/run_unit_tests.sh CMBUILD=${CMAKE_CURRENT_BINARY_DIR}
		WORKING_DIRECTORY ../test
)

# cmake seems unable to start test/app_test/run_all.ksh, so we have to lump
# a dummy script in ./test that does the obvious thing.
add_test(
		NAME drive_app
		COMMAND bash  ./run_app_tests.sh LD_LIBRARY_PATH=${install_root}/lib C_INCLUDE_PATH=${install_root}/include CMBUILD=${CMAKE_CURRENT_BINARY_DIR}
		WORKING_DIRECTORY ../test
)


# --- support binaries that depend on the libs identified above ---------------
add_subdirectory( src/support )

# ------------- packaging -----------------------------------------------------

#
if( BUILD_NNG )
	if( APPLE  )
		message( "### apple hack: forcing hard coded library paths for nng/nano dynamic libraries" )
		target_link_libraries( rmr_nng_shared ${CMAKE_CURRENT_BINARY_DIR}/lib/libnng${nng_so_suffix} )
	endif()
endif()

# Define what should be installed, and where they should go. For dev package we install
# only the RMr headers, man pages and archive (.a) files.  The run-time package gets just
# the library (.so) files and nothing more.
#
if( BUILD_NNG )
	if( DEV_PKG )
		set( target_list "rmr_nng_static;rmr_si_static" )
	else()
		set( target_list "rmr_nng_shared;rmr_si_shared" )
	endif()
else()
	if( DEV_PKG )
		set( target_list "rmr_si_static" )
	else()
		set( target_list "rmr_si_shared" )
	endif()
endif()

install( TARGETS ${target_list} EXPORT LibraryConfig
	LIBRARY  DESTINATION ${install_lib}
	ARCHIVE  DESTINATION ${install_lib}
	PUBLIC_HEADER DESTINATION ${install_inc}
)


unset( DEV_PKG  CACHE )			# prevent from being a hidden setting if user redoes things

# install any nano/nng libraries in to the deb as well, but ONLY if asked for on the 'cmake ..' command
# (sure would be nice if FILEs allowed for globbing; sadly it does not.) Disabled by default if BUILD_NNG
# is turned off.
#
if( PACK_EXTERNALS )
	message( "+++ including nano and nng libraries in the deb" )
	install( FILES
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnng${nng_so_suffix}
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnng${nng_so_suffix_m}
		${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/libnng${nng_so_suffix_mm}

		DESTINATION ${install_lib}
	)
endif()

unset( SKIP_EXTERNALS  CACHE )	# prevent these from being applied next build unless specifically set on comd line
unset( PACK_EXTERNALS  CACHE )

IF( EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake" )
	include( InstallRequiredSystemLibraries )

	set( CPACK_DEBIAN_PACKAGE_NAME ${deb_pkg_name} )
	set( CPACK_RPM_PACKAGE_NAME ${rpm_pkg_name} )

	# auto dependency checking makes the RPM install fail, see NOTES
	set( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION="/usr/local /usr/local/bin /usr/local/lib" )

	set( CPACK_set_DESTDIR "on" )
	set( CPACK_PACKAGING_INSTALL_PREFIX "${install_root}" )
	set( CPACK_GENERATOR "${pkg_list}" )

	set( CPACK_PACKAGE_DESCRIPTION "Thin library for RIC xAPP messaging routed based on message type." )
	set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "RIC message routing library" )
	set( CPACK_PACKAGE_VENDOR "None" )
	set( CPACK_PACKAGE_CONTACT "None" )
	set( CPACK_PACKAGE_VERSION_MAJOR "${major_version}" )
	set( CPACK_PACKAGE_VERSION_MINOR "${minor_version}" )
	set( CPACK_PACKAGE_VERSION_PATCH "${patch_level}" )
	set( CPACK_PACKAGE "${pkg_label}" )						# generic name for old versions of cpack
	set( CPACK_DEBIAN_FILE_NAME "${deb_pkg_label}.deb" )
	set( CPACK_RPM_FILE_NAME "${rpm_pkg_label}.rpm" )

	# there is not an NNG package, so we cannot define a dependency

	set( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )
	set( CPACK_DEBIAN_PACKAGE_SECTION "ric" )
	set( CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )
	set( CPACK_RPM_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )

	# this seems ingnored if included
	#set( CPACK_COMPONENTS_ALL Libraries ApplicationData )

	INCLUDE( CPack )
ENDIF()
