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

libsmartbookmarksextension_la_LDFLAGS = -module -avoid-version

extensionxmldir = $(extensiondir)
extensionxml_in_files = smart-bookmarks.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

CLEANFILES = $(extensionxml_DATA)
DISTCLEANFILES = $(extensionxml_DATA)

EXTRA_DIST = $(glade_DATA) $(extensionxml_in_files)
