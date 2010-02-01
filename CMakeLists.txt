# define minimum cmake version
cmake_minimum_required(VERSION 2.4)
 
project( Guayadeque )

IF( NOT EXISTS ${CMAKE_INSTALL_PREFIX} )
	SET( CMAKE_INSTALL_PREFIX  "/usr" )
ENDIF( NOT EXISTS ${CMAKE_INSTALL_PREFIX} )

MESSAGE( STATUS "Using install prefix ${CMAKE_INSTALL_PREFIX} ..." )

FIND_PACKAGE(PkgConfig)

SET( 	SEARCH_LIBS
	/usr/lib
	/usr/local/lib )


#SET(CMAKE_FIND_LIBRARY_PREFIXES "")
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

SET(wxWidgets_USE_LIBS base core adv net html xml aui)

FIND_PACKAGE(wxWidgets)

IF( NOT wxWidgets_FOUND)
    MESSAGE( FATAL_ERROR "wxWidgets not found!" )
ENDIF( NOT wxWidgets_FOUND)

PKG_CHECK_MODULES( GSTREAMER gstreamer-0.10 )
IF( NOT GSTREAMER_LIBRARIES )
	MESSAGE( FATAL_ERROR "gstreamer-0.10 not found!" )
ENDIF( NOT GSTREAMER_LIBRARIES )

PKG_CHECK_MODULES( SQLITE3 sqlite3 )
IF( NOT SQLITE3_LIBRARIES )
	MESSAGE( FATAL_ERROR "sqlite3 not found!" )
ENDIF( NOT SQLITE3_LIBRARIES )

PKG_CHECK_MODULES( LIBCURL libcurl )
IF( NOT LIBCURL_LIBRARIES )
	MESSAGE( FATAL_ERROR "libcurl not found!" )
ENDIF( NOT LIBCURL_LIBRARIES )

PKG_CHECK_MODULES( LIBTAG taglib )
IF( NOT LIBTAG_LIBRARIES )
	MESSAGE( FATAL_ERROR "taglib not found!" )
ENDIF( NOT LIBTAG_LIBRARIES )


#PKG_CHECK_MODULES( LIBTAGEXTRAS taglib-extras )
#IF( NOT LIBTAGEXTRAS_LIBRARIES )
#	MESSAGE( FATAL_ERROR "taglib-extras not found!" )
#ENDIF( NOT LIBTAGEXTRAS_LIBRARIES )

PKG_CHECK_MODULES( LIBDBUS dbus-1 )
IF( NOT LIBDBUS_LIBRARIES )
	MESSAGE( FATAL_ERROR "dbus not found!" )
ENDIF( NOT LIBDBUS_LIBRARIES )

PKG_CHECK_MODULES( LIBFLAC flac )
IF( NOT LIBFLAC_LIBRARIES )
	MESSAGE( FATAL_ERROR "libflac not found!" )
ENDIF( NOT LIBFLAC_LIBRARIES )


# Include wxWidgets macros
INCLUDE(${wxWidgets_USE_FILE})

#    
#ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
ADD_DEFINITIONS( -Wall -O2 )
 
# We define the include paths here, our minimal source dir is one, 
# and also the include dirs defined by wxWidgets
INCLUDE_DIRECTORIES(${Guayadeque_SOURCE_DIR}
			${Guayadeque_SOURCE_DIR}/src
                        ${wxWidgets_INCLUDE_DIRS}
                        ${GSTREAMER_INCLUDE_DIRS}
			${SQLITE3_INCLUDE_DIRS}
			${LIBCURL_INCLUDE_DIRS}
			${LIBTAG_INCLUDE_DIRS}
			${LIBMP4_INCLUDE_DIRS}
              		${LIBDBUS_INCLUDE_DIRS}
                         )
 
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( po )

# default configuration and desktop icon
INSTALL( FILES guayadeque.desktop DESTINATION share/applications )
INSTALL( FILES guayadeque.png DESTINATION share/pixmaps/ )
INSTALL( FILES guayadeque.default.conf DESTINATION share/guayadeque )
INSTALL( FILES equalizers.default.conf DESTINATION share/guayadeque )

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

