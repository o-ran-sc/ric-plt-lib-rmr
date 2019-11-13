
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


# for clarity: this generates object, not a lib as the CM command implies. We also tag
# them as rmr_si objects inasmuch as we might maintain the si code somewhere in this
# repo as well.
#
add_library( rmr_si_objects OBJECT 
	src/rmr_si.c 

	src/si95/siaddress.c
	src/si95/sibldpoll.c
	src/si95/sicbreg.c
	src/si95/sicbstat.c
	src/si95/siclose.c
	src/si95/siconnect.c
	src/si95/siestablish.c
	src/si95/sigetadd.c
	src/si95/sigetname.c
	src/si95/siinit.c
	src/si95/silisten.c
	src/si95/sinew.c
	src/si95/sinewses.c
	src/si95/sipoll.c
	src/si95/sircv.c
	src/si95/sisend.c
	src/si95/sisendt.c
	src/si95/sishutdown.c
	src/si95/siterm.c
	src/si95/sitrash.c
	src/si95/siwait.c
)

#if( need_ext )
#	add_dependencies( nano_objects si )		# force external things to build first
#endif()

target_include_directories (rmr_si_objects PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common/src>
	$<INSTALL_INTERFACE:include>
	PRIVATE src)

