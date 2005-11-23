SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libpushscrollerextension.la

libpushscrollerextension_la_SOURCES = \
	ephy-push-scroller-extension.c	\
	ephy-push-scroller-extension.h	\
	ephy-push-scroller.c		\
	ephy-push-scroller.h		\
	extension.c

libpushscrollerextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/push-scroller/mozilla	\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

libpushscrollerextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libpushscrollerextension_la_LIBADD = \
	mozilla/libpushscrollermozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)
libpushscrollerextension_la_LDFLAGS = -module -avoid-version

extensioninidir = $(extensiondir)
extensionini_in_files = push-scroller.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(extensionini_in_files)
