SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	-I$(top_srcdir)/extensions/page-info/mozilla		\
	-I$(top_srcdir)/extensions/page-info			\
	-I$(top_srcdir)/include					\
	$(EPIPHANY_DEPENDENCY_CFLAGS)				\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED					\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED				\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libpageinfoextension.la

libpageinfoextension_la_SOURCES = \
	ephy-page-info-extension.c	\
	ephy-page-info-extension.h	\
	page-info-dialog.c		\
	page-info-dialog.h		\
	extension.c

libpageinfoextension_la_LIBADD = \
	mozilla/libpageinfomozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

glade_DATA = page-info.glade

gladedir = $(pkgdatadir)/glade

ui_xml_DATA = page-info-context-ui.xml

ui_xmldir = $(pkgdatadir)/xml

extensionxmldir = $(extensiondir)
extensionxml_in_files = page-info.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`source $(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

BUILT_SOURCES = $(extensionxml_DATA)

CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(glade_DATA) $(ui_xml_DATA) $(extensionxml_in_files)
