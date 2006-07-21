SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = liblivehttpheadersextension.la

liblivehttpheadersextension_la_SOURCES = \
	ephy-livehttpheaders-extension.c	\
	ephy-livehttpheaders-extension.h	\
	livehttpheaders-ui.c			\
	livehttpheaders-ui.h			\
	extension.c

liblivehttpheadersextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/livehttpheaders/mozilla	\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

liblivehttpheadersextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

gladedir = $(pkgdatadir)/glade
glade_DATA = livehttpheaders-ui.glade

liblivehttpheadersextension_la_LIBADD = \
	mozilla/liblivehttpheadersmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)
liblivehttpheadersextension_la_LDFLAGS = -module -avoid-version

extensioninidir = $(extensiondir)
extensionini_in_files = livehttpheaders.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = \
	$(extensionini_in_files) \
	$(glade_DATA)

