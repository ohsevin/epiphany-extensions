extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
        -I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DEPHY_PLUGINS_LOCALEDIR=\"$(datadir)/locale\"  \
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libextensionsmanageruiextension.la

libextensionsmanageruiextension_la_SOURCES = \
	extensions-manager-ui.c \
	extensions-manager-ui.h \
	ephy-extensions-manager-ui-extension.c	\
	ephy-extensions-manager-ui-extension.h	\
	extension.c

gladedir = $(pkgdatadir)/glade
glade_DATA = extensions-manager-ui.glade

extensionxmldir = $(extensiondir)
extensionxml_in_files = extensions-manager-ui.xml.in.in
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

EXTRA_DIST = $(glade_DATA) $(extensionxml_in_files)
