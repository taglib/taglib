DEFS = -DMAKE_TAGLIB_LIB @DEFS@
INCLUDES = \
	-I$(top_srcdir)/taglib \
	-I$(top_srcdir)/taglib/toolkit \
	$(all_includes)

noinst_LTLIBRARIES = libape.la

libape_la_SOURCES = apetag.cpp apefooter.cpp apeitem.cpp apefile.cpp apeproperties.cpp

taglib_include_HEADERS = apetag.h apefooter.h apeitem.h apefile.h apeproperties.cpp
taglib_includedir = $(includedir)/taglib
