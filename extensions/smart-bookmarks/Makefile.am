SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libsmartbookmarksextension.la

libsmartbookmarksextension_la_SOURCES =	\
	smart-bookmarks-extension.c	\
	smart-bookmarks-extension.h	\
        smart-bookmarks.c

libsmartbookmarksextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/smart-bookmarks/mozilla	\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
        -DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	$(AM_CPPFLAGS)

libsmartbookmarksextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libsmartbookmarksextension_la_LIBADD =	\
	$(top_builddir)/extensions/smart-bookmarks/mozilla/libsbmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

libsmartbookmarksextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

extensioninidir = $(extensiondir)
extensionini_in_files = smart-bookmarks.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(glade_DATA) $(extensionini_in_files)
