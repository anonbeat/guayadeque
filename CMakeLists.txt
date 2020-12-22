# define minimum cmake version
cmake_minimum_required(VERSION 3.0)
IF(POLICY CMP0005)
   cmake_policy(SET CMP0005 NEW)
ENDIF(POLICY CMP0005)

IF(POLICY CMP0037)
   cmake_policy(SET CMP0037 OLD)   # cmake warns about sr@latin target
ENDIF(POLICY CMP0037)
 
project( Guayadeque )

# Set Release type for builds where CMAKE_BUILD_TYPE is unset.
# This is usually a good default as this implictly enables
#
#   CFLAGS = -O3 -DNDEBUG
#
IF( NOT CMAKE_BUILD_TYPE )
        SET( CMAKE_BUILD_TYPE "Release" )
ENDIF( NOT CMAKE_BUILD_TYPE )

# Adhere to GNU filesystem layout conventions
include( GNUInstallDirs )

MESSAGE( STATUS "Using install prefix ${CMAKE_INSTALL_PREFIX} ..." )

FIND_PACKAGE(PkgConfig)

SET( SEARCH_LIBS
        /usr/lib
        /usr/local/lib )

# [sl 2020-05-23] according to https://cmake.org/cmake/help/v3.0/module/FindwxWidgets.html
SET(wxWidgets_CONFIG_OPTIONS --toolkit=gtk3)

FIND_PACKAGE( wxWidgets COMPONENTS base core adv net html xml aui qa )

IF( NOT wxWidgets_FOUND)
    MESSAGE( FATAL_ERROR "wxWidgets not found!" )
ENDIF( NOT wxWidgets_FOUND)

PKG_CHECK_MODULES( GSTREAMER REQUIRED gstreamer-1.0 )
PKG_CHECK_MODULES( GSTREAMER_TAG REQUIRED gstreamer-tag-1.0 )
PKG_CHECK_MODULES( SQLITE3 REQUIRED sqlite3 )
PKG_CHECK_MODULES( LIBCURL REQUIRED libcurl )

PKG_CHECK_MODULES( LIBTAG taglib>=1.6.4)
IF( NOT LIBTAG_FOUND )
        PKG_CHECK_MODULES( LIBTAG taglib>=1.6.1)
        IF( NOT LIBTAG_FOUND )
                PKG_CHECK_MODULES( LIBTAG taglib>=1.6 )
                IF( NOT LIBTAG_FOUND )
                        MESSAGE( FATAL_ERROR "taglib 1.6 not found!" )
                ENDIF( NOT LIBTAG_FOUND )
        ELSE( NOT LIBTAG_FOUND )
                ADD_DEFINITIONS(-DTAGLIB_WITH_MP4_COVERS=1)
        ENDIF( NOT LIBTAG_FOUND )
ELSE( NOT LIBTAG_FOUND )
        ADD_DEFINITIONS(-DTAGLIB_WITH_MP4_COVERS=1)
        ADD_DEFINITIONS(-DTAGLIB_WITH_APE_SUPPORT=1)
ENDIF( NOT LIBTAG_FOUND )

PKG_CHECK_MODULES( LIBDBUS REQUIRED dbus-1 )

PKG_CHECK_MODULES( LIBGDKPIXBUF20 REQUIRED gdk-pixbuf-2.0 )
PKG_CHECK_MODULES( LIBGIO REQUIRED gio-2.0 )

OPTION( ENABLE_IPOD "Enable iPod support by including libgpod support" ON )
IF( ENABLE_IPOD )
        PKG_CHECK_MODULES( LIBGPOD libgpod-1.0 )
        IF( NOT LIBGPOD_FOUND )
                MESSAGE( " libgpod-dev not found! No iPod support!!" )
        ELSE( NOT LIBGPOD_FOUND )
                ADD_DEFINITIONS(-DWITH_LIBGPOD_SUPPORT=1)
        ENDIF( NOT LIBGPOD_FOUND )
ENDIF( ENABLE_IPOD )

OPTION( ENABLE_LIBINDICATE "Enable support for Ayatana notification using the libindicate library" ON )
IF( ENABLE_LIBINDICATE )
        PKG_CHECK_MODULES( LIBINDICATE07 indicate-0.7 )
        IF( NOT LIBINDICATE07_FOUND )
                PKG_CHECK_MODULES( LIBINDICATE06 indicate-0.6 )
                IF( NOT LIBINDICATE06_FOUND )
                        PKG_CHECK_MODULES( LIBINDICATE indicate )
                        IF( NOT LIBINDICATE_FOUND )
                                MESSAGE( " libindicate library not found! No sound menu support!!" )
                        ELSE( NOT LIBINDICATE_FOUND )
                                ADD_DEFINITIONS(-DWITH_LIBINDICATE_SUPPORT=1)
                        ENDIF( NOT LIBINDICATE_FOUND )
                ELSE( NOT LIBINDICATE06_FOUND )
                        ADD_DEFINITIONS(-DWITH_LIBINDICATE_SUPPORT=1)
                ENDIF( NOT LIBINDICATE06_FOUND )
        ELSE( NOT LIBINDICATE07_FOUND )
                ADD_DEFINITIONS(-DWITH_LIBINDICATE_SUPPORT=1)
        ENDIF( NOT LIBINDICATE07_FOUND )
ENDIF( ENABLE_LIBINDICATE )

PKG_CHECK_MODULES( LIBWXSQLITE3 wxsqlite3 )
IF( NOT LIBWXSQLITE3_FOUND )
        PKG_CHECK_MODULES( LIBWXSQLITE330 wxsqlite3-3.0 )
        IF( NOT LIBWXSQLITE330_FOUND )
                MESSAGE( FATAL_ERROR "wxSqlite3 not found" )
        ENDIF( NOT LIBWXSQLITE330_FOUND )
ENDIF( NOT LIBWXSQLITE3_FOUND )

PKG_CHECK_MODULES( LIBJSONCPP jsoncpp )
IF( NOT LIBJSONCPP_FOUND )
  	MESSAGE( FALTAL_ERROR "libjsoncpp not found" )
ENDIF( NOT LIBJSONCPP_FOUND )

# Include wxWidgets macros
INCLUDE(${wxWidgets_USE_FILE})

IF( NOT _GUREVISION_ )
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
      COMMAND git log -1 --format=%h
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE _GUREVISION_
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
ENDIF( NOT _GUREVISION_ )

# Make Version.h
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/Version.h )

#    
#ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
ADD_COMPILE_OPTIONS( -Wall )
ADD_DEFINITIONS( -DDATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}/guayadeque\" -DDATAROOTDIR=\"${CMAKE_INSTALL_FULL_DATAROOTDIR}\" )
 
# We define the include paths here, our minimal source dir is one, 
# and also the include dirs defined by wxWidgets
INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${wxWidgets_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER_TAG_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${LIBCURL_INCLUDE_DIRS}
        ${LIBTAG_INCLUDE_DIRS}
        ${LIBDBUS_INCLUDE_DIRS}
        ${LIBGIO_INCLUDE_DIRS}
        ${LIBGPOD_INCLUDE_DIRS}
        ${LIBINDICATE_INCLUDE_DIRS}
        ${LIBINDICATE06_INCLUDE_DIRS}
        ${LIBINDICATE07_INCLUDE_DIRS}
        ${LIBWXSQLITE3_INCLUDE_DIRS}
        ${LIBWXSQLITE330_INCLUDE_DIRS}
	${LIBJSONCPP_INCLUDE_DIRS}
)

IF( PROFILE )
        ADD_DEFINITIONS( -pg )
        SET(CMAKE_EXE_LINKER_FLAGS "-pg -g")
        MESSAGE( "Adding profiling code." )
ENDIF( PROFILE )

 
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( po )

# default configuration and desktop icon
# freedesktop.org-specific files
INSTALL( FILES defconfig/guayadeque.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )
INSTALL( FILES defconfig/guayadeque.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps )
INSTALL( FILES defconfig/guayadeque.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/appdata )

# program-specific files
INSTALL( FILES defconfig/guayadeque.default.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/guayadeque )
INSTALL( FILES defconfig/equalizers.default.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/guayadeque )
INSTALL( FILES defconfig/lyrics_sources.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/guayadeque )

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

ADD_CUSTOM_TARGET(debug
  ${CMAKE_COMMAND} -DMAKE_BUILD_TYPE:STRING=Debug ${PROJECT_SOURCE_DIR})

ADD_CUSTOM_TARGET(release
  ${CMAKE_COMMAND} -DMAKE_BUILD_TYPE:STRING=Release ${PROJECT_SOURCE_DIR})

