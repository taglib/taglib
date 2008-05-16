INCLUDES = \
	-I$(top_srcdir)/taglib \
	-I$(top_srcdir)/taglib/toolkit \
	-I$(top_srcdir)/taglib/riff \
	-I$(top_srcdir)/taglib/mpeg/id3v2 \
	$(all_includes)

noinst_LTLIBRARIES = libaiff.la

libaiff_la_SOURCES = aifffile.cpp

taglib_include_HEADERS = aifffile.h
taglib_includedir = $(includedir)/taglib
