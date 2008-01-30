SUBDIRS = toolkit mpeg ogg flac ape mpc wavpack speex trueaudio

INCLUDES = \
	-I$(top_srcdir)/taglib \
	-I$(top_srcdir)/taglib/toolkit \
	-I$(top_srcdir)/taglib/mpeg \
	-I$(top_srcdir)/taglib/ogg \
	-I$(top_srcdir)/taglib/ogg/flac \
	-I$(top_srcdir)/taglib/flac \
	-I$(top_srcdir)/taglib/mpc \
	-I$(top_srcdir)/taglib/ogg/vorbis \
	-I$(top_srcdir)/taglib/wavpack \
	-I$(top_srcdir)/taglib/speex \
	-I$(top_srcdir)/taglib/trueaudio \
	$(all_includes)

lib_LTLIBRARIES = libtag.la

libtag_la_SOURCES = tag.cpp fileref.cpp audioproperties.cpp
taglib_include_HEADERS = tag.h fileref.h audioproperties.h taglib_export.h
taglib_includedir = $(includedir)/taglib

libtag_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 6:0:5
libtag_la_LIBADD = ./mpeg/libmpeg.la ./ogg/libogg.la ./flac/libflac.la ./mpc/libmpc.la \
	./ape/libape.la ./toolkit/libtoolkit.la ./wavpack/libwavpack.la ./speex/libspeex.la \
	./trueaudio/libtrueaudio.la
