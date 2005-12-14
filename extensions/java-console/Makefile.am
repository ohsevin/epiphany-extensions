SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libjavaconsoleextension.la

libjavaconsoleextension_la_SOURCES = \
	ephy-java-console-extension.c	\
	ephy-java-console-extension.h	\
	java-console.c

libjavaconsoleextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/java-console/mozilla	\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

libjavaconsoleextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libjavaconsoleextension_la_LIBADD = \
	mozilla/libjavaconsolemozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

libjavaconsoleextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

extensioninidir = $(extensiondir)
extensionini_in_files = java-console.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(extensionini_in_files)
