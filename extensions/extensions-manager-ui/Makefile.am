extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libextensionsmanageruiextension.la

libextensionsmanageruiextension_la_SOURCES = \
	extensions-manager-ui.c \
	extensions-manager-ui.h \
	ephy-extensions-manager-ui-extension.c	\
	ephy-extensions-manager-ui-extension.h	\
	extension.c
libextensionsmanageruiextension_la_LDFLAGS = -module -avoid-version

libextensionsmanageruiextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libextensionsmanageruiextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

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
