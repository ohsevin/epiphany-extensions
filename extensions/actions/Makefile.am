extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libactionsextension.la

libactionsextension_la_SOURCES = \
	ephy-actions-extension-editor-dialog.c		\
	ephy-actions-extension-editor-dialog.h		\
	ephy-actions-extension-properties-dialog.c	\
	ephy-actions-extension-properties-dialog.h	\
	ephy-actions-extension-util.c			\
	ephy-actions-extension-util.h			\
	ephy-actions-extension.c			\
	ephy-actions-extension.h			\
	extension.c

libactionsextension_la_CPPFLAGS = \
	-I$(top_srcdir)/include					\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	$(AM_CPPFLAGS)

libactionsextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

glade_DATA = \
	action-properties.glade	\
	actions-editor.glade
gladedir = $(pkgdatadir)/glade

extensionxmldir = $(extensiondir)
extensionxml_in_files = actions.xml.in.in
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
