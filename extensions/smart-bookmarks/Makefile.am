extensiondir = $(EXTENSIONS_DIR)

SUBDIRS = mozilla

INCLUDES = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/smart-bookmarks/mozilla	\
	$(EPIPHANY_DEPENDENCY_CFLAGS)				\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
        -DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED					\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED				\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libsmartbookmarksextension.la

libsmartbookmarksextension_la_SOURCES =	\
	smart-bookmarks-extension.c	\
	smart-bookmarks-extension.h	\
	smart-bookmarks-prefs.h		\
	smart-bookmarks-prefs-ui.c	\
	smart-bookmarks-prefs-ui.h	\
        smart-bookmarks.c
	
libsmartbookmarksextension_la_LIBADD =	\
	mozilla/libsbmozilla.la		\
	$(MOZILLA_COMPONENT_LIBS)

glade_DATA = smart-bookmarks.glade

gladedir = $(pkgdatadir)/glade

extensionxmldir = $(extensiondir)
extensionxml_in_files = smart-bookmarks.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(LOCALE_DIR)|" \
	    -e "s|%LIBRARY%|FIXME|" \
	$< > $@

@INTLTOOL_XML_RULE@

BUILT_SOURCES = $(extensionxml_DATA)

DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(glade_DATA) $(extensionxml_in_files)
