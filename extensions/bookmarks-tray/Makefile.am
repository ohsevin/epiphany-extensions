extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libbookmarkstrayextension.la

libbookmarkstrayextension_la_SOURCES = \
	ephy-bookmarks-tray-extension.c	\
	ephy-bookmarks-tray-extension.h	\
	extension.c

libbookmarkstrayextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

libbookmarkstrayextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

ui_xmldir = $(pkgdatadir)/xml
ui_xml_DATA = ephy-bookmarks-tray-ui.xml

extensionxmldir = $(extensiondir)
extensionxml_in_files = bookmarks-tray.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`source $(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

CLEANFILES = $(extensionxml_DATA)
DISTCLEANFILES = $(extensionxml_DATA)

EXTRA_DIST = \
	$(ui_xml_DATA)			\
	$(extensionxml_in_files)
