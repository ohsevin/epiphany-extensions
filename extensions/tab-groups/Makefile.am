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

extension_LTLIBRARIES = libtabgroupsextension.la

libtabgroupsextension_la_SOURCES = \
	ephy-tab-groups-extension.c	\
	ephy-tab-groups-extension.h	\
	ephy-tab-grouper.c		\
	ephy-tab-grouper.h		\
	tab-groups.c

extensionxmldir = $(extensiondir)
extensionxml_in_files = tab-groups.xml.in.in
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
