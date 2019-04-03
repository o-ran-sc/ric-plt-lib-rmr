# Install script for directory: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/src/nanomsg.pc")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnanomsg.so.5.1.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnanomsg.so.5"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnanomsg.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/lib")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/libnanomsg.so.5.1.0"
    "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/libnanomsg.so.5"
    "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/libnanomsg.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnanomsg.so.5.1.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnanomsg.so.5"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnanomsg.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
           NEW_RPATH "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/lib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/nanomsg-1.1.5-1-ge40de97/nanomsg-target.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/nanomsg-1.1.5-1-ge40de97/nanomsg-target.cmake"
         "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/src/CMakeFiles/Export/lib/cmake/nanomsg-1.1.5-1-ge40de97/nanomsg-target.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/nanomsg-1.1.5-1-ge40de97/nanomsg-target-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/nanomsg-1.1.5-1-ge40de97/nanomsg-target.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/nanomsg-1.1.5-1-ge40de97" TYPE FILE FILES "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/src/CMakeFiles/Export/lib/cmake/nanomsg-1.1.5-1-ge40de97/nanomsg-target.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/nanomsg-1.1.5-1-ge40de97" TYPE FILE FILES "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/src/CMakeFiles/Export/lib/cmake/nanomsg-1.1.5-1-ge40de97/nanomsg-target-noconfig.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/nanomsg-1.1.5-1-ge40de97" TYPE FILE FILES
    "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/src/nanomsg-config.cmake"
    "/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/src/nanomsg-config-version.cmake"
    )
endif()

