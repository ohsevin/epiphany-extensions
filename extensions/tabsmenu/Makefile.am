extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	-I$(top_srcdir)/include 			\
	$(EPIPHANY_DEPENDENCY_CFLAGS)			\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_PLUGINS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED			\
	-DGTK_DISABLE_DEPRECATED			\
	-DGDK_PIXBUF_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libtabsmenuextension.la

libtabsmenuextension_la_SOURCES = \
	ephy-tabs-menu-extension.c	\
	ephy-tabs-menu-extension.h	\
	ephy-tab-move-menu.c		\
	ephy-tab-move-menu.h		\
	tabs-menu.c

extensionxmldir = $(extensiondir)
extensionxml_in_files = tabsmenu.xml.in.in
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

EXTRA_DIST = $(extensionxml_in_files)
