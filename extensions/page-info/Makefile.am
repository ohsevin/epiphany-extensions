SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libpageinfoextension.la

libpageinfoextension_la_SOURCES = \
	ephy-page-info-extension.c	\
	ephy-page-info-extension.h	\
	page-info-dialog.c		\
	page-info-dialog.h		\
	extension.c

libpageinfoextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-I$(top_srcdir)/extensions/page-info/mozilla	\
	-I$(MOZILLA_INCLUDE_ROOT)/gtkembedmoz		\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libpageinfoextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libpageinfoextension_la_LIBADD = \
	mozilla/libpageinfomozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)
libpageinfoextension_la_LDFLAGS = -module -avoid-version

glade_DATA = page-info.glade
gladedir = $(pkgdatadir)/glade

ui_xml_DATA = page-info-context-ui.xml
ui_xmldir = $(pkgdatadir)/xml

extensionxmldir = $(extensiondir)
extensionxml_in_files = page-info.xml.in.in
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

EXTRA_DIST = $(glade_DATA) $(ui_xml_DATA) $(extensionxml_in_files)
