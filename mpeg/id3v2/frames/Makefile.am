INCLUDES = \
	-I$(top_srcdir)/taglib \
	-I$(top_srcdir)/taglib/toolkit \
	-I$(top_srcdir)/taglib/mpeg/id3v2 \
	$(all_includes)

noinst_LTLIBRARIES = libframes.la

libframes_la_SOURCES = \
	attachedpictureframe.cpp \
	commentsframe.cpp \
	relativevolumeframe.cpp \
	textidentificationframe.cpp \
	uniquefileidentifierframe.cpp \
	unknownframe.cpp

taglib_include_HEADERS = \
	attachedpictureframe.h \
	commentsframe.h \
	relativevolumeframe.h \
	textidentificationframe.h \
	uniquefileidentifierframe.h \
	unknownframe.h

taglib_includedir = $(includedir)/taglib

EXTRA_DIST = $(libframes_la_SOURCES) $(taglib_include_HEADERS)
