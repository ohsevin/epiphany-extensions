extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
        -I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
        -DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"       \
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libtoolbarextrasextension.la

libtoolbarextrasextension_la_SOURCES = \
	ephy-toolbar-extras-extension.h	\
	ephy-toolbar-extras-extension.c	\
	ephy-multi-smart-action.h	\
	ephy-multi-smart-action.c	\
	galago-gtk-icon-entry.c		\
	galago-gtk-icon-entry.h		\
	toolbar-extras.c

extensionxmldir = $(extensiondir)
extensionxml_in_files = toolbar-extras.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(LOCALE_DIR)|" \
	    -e "s|%LIBRARY%|FIXME|" \
	$< > $@

@INTLTOOL_XML_RULE@

BUILT_SOURCES = $(extensionxml_DATA)

DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(extensionxml_in_files)
