
FIND_PACKAGE( Gettext REQUIRED )

MACRO(GETTEXT_PROCESS_POFILE _lang _moFile _poFile )
   GET_FILENAME_COMPONENT(_moBasename ${_moFile} NAME_WE)
   GET_FILENAME_COMPONENT(_absmoFile ${_moFile} ABSOLUTE)
   GET_FILENAME_COMPONENT(_absFile ${_poFile} ABSOLUTE)
   SET(_gmoFile ${_absmoFile})
   ADD_CUSTOM_COMMAND(
      OUTPUT ${_gmoFile}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_absmoFile} ${_absFile}
      #DEPENDS ${_absmoFile} ${_absFile}
   )
   INSTALL(FILES ${_gmoFile} DESTINATION share/locale/${_lang}/LC_MESSAGES )
   #ADD_CUSTOM_TARGET( translations ALL DEPENDS ${_gmoFile})
   ADD_CUSTOM_TARGET( ${_moBasename}.${_lang}.mo ALL DEPENDS ${_gmoFile} )
ENDMACRO(GETTEXT_PROCESS_POFILE )

# Current languages
ADD_SUBDIRECTORY( es )
ADD_SUBDIRECTORY( uk )
ADD_SUBDIRECTORY( it )
ADD_SUBDIRECTORY( de )
ADD_SUBDIRECTORY( fr )
ADD_SUBDIRECTORY( is )
ADD_SUBDIRECTORY( no )

