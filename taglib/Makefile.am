SUBDIRS = toolkit mpeg ogg flac ape mpc

INCLUDES = \
	-I$(top_srcdir)/taglib/toolkit \
	-I$(top_srcdir)/taglib/mpeg \
	-I$(top_srcdir)/taglib/ogg \
	-I$(top_srcdir)/taglib/flac \
	-I$(top_srcdir)/taglib/mpc \
	-I$(top_srcdir)/taglib/ogg/vorbis \
	$(all_includes)

lib_LTLIBRARIES = libtag.la

libtag_la_SOURCES = tag.cpp fileref.cpp audioproperties.cpp
taglib_include_HEADERS = tag.h fileref.h audioproperties.h
taglib_includedir = $(includedir)/taglib

libtag_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 5:0:4
libtag_la_LIBADD = ./mpeg/libmpeg.la ./ogg/libogg.la ./flac/libflac.la ./mpc/libmpc.la ./ape/libape.la ./toolkit/libtoolkit.la

bin_SCRIPTS = taglib-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = taglib.pc

EXTRA_DIST = $(libtag_la_SOURCES) $(taglib_include_HEADERS) $(pkgconfig_DATA)

examples: examples-all

examples-all:
	cd examples ; \
	$(MAKE) all

apidox:
	$(mkinstalldirs) doc/api; \
	if test ! -x doc/common; then \
		$(LN_S) $(kde_libs_htmldir)/en/common doc/common ; \
	fi; \
	cp $(top_srcdir)/admin/Doxyfile.global taglib.doxyfile; \
	echo "PROJECT_NAME        = TagLib" >> taglib.doxyfile; \
	echo "PROJECT_NUMBER      = \"Version 1.1\"" >> taglib.doxyfile; \
	echo "INPUT               = $(srcdir)" >> taglib.doxyfile; \
	echo "OUTPUT_DIRECTORY    = doc/api" >> taglib.doxyfile; \
	echo "HTML_OUTPUT         = html" >> taglib.doxyfile; \
	echo "GENERATE_HTML       = YES" >> taglib.doxyfile ; \
	echo "GENERATE_MAN        = NO" >> taglib.doxyfile ; \
	echo "GENERATE_LATEX      = NO" >> taglib.doxyfile ; \
	echo "HTML_HEADER         = doc/common/header.html" >> taglib.doxyfile ; \
	echo "HTML_FOOTER         = doc/common/footer.html" >> taglib.doxyfile ; \
	echo "HTML_STYLESHEET     = doc/common/doxygen.css" >> taglib.doxyfile ; \
	echo "FILE_PATTERNS       = *.h" >> taglib.doxyfile ; \
	echo "PREDEFINED          = DO_NOT_DOCUMENT DOXYGEN" >> taglib.doxyfile ; \
	echo "EXTRACT_ALL         = YES" >> taglib.doxyfile ; \
	doxygen taglib.doxyfile
