include(CheckTypeSize)

check_type_size("int" SIZEOF_INT)
check_type_size("long" SIZEOF_LONG)

include(CheckIncludeFile)

check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("strings.h" HAVE_STRINGS_H)
check_include_file("unistd.h" HAVE_UNISTD_H)


CONFIGURE_FILE( 
	${CMAKE_CURRENT_SOURCE_DIR}/config-taglib-extras.h.cmake 
	${CMAKE_CURRENT_SOURCE_DIR}/config-taglib-extras.h 
)

