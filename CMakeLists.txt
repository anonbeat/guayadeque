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

SET(wxWidgets_USE_LIBS base core adv net html xml media)

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

FIND_LIBRARY( LIBID3_LIBRARIES NAMES "id3" PATHS ${SEARCH_LIBS} )
IF( NOT LIBID3_LIBRARIES )
	MESSAGE( FATAL_ERROR "Unable to locate libid3 library file!" )
ENDIF( NOT LIBID3_LIBRARIES )

    # Include wxWidgets macros
    INCLUDE(${wxWidgets_USE_FILE})

    #    
    ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
 
    # We define the include paths here, our minimal source dir is one, 
    # and also the include dirs defined by wxWidgets
    INCLUDE_DIRECTORIES(${Guayadeque_SOURCE_DIR}
			${Guayadeque_SOURCE_DIR}/src
                        ${wxWidgets_INCLUDE_DIRS}
                        ${GSTREAMER_INCLUDE_DIRS}
			${SQLITE3_INCLUDE_DIRS}
			${LIBCURL_INCLUDE_DIRS}
                         )
 
    subdirs( src )
 
