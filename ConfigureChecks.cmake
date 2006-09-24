# NOTE: only add something here if it is really needed by all of kdelibs.
#     Otherwise please prefer adding to the relevant config-foo.h.cmake file,
#     and the CMakeLists.txt that generates it (or a separate ConfigureChecks.make file if you prefer)
#     to minimize recompilations and increase modularity.

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

#check for libz using the cmake supplied FindZLIB.cmake

IF(ZLIB_FOUND)
	SET(HAVE_LIBZ 1)
ELSE(ZLIB_FOUND)
	SET(HAVE_LIBZ 0)
ENDIF(ZLIB_FOUND)
