extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libtabsmenuextension.la

libtabsmenuextension_la_SOURCES = \
	ephy-tabs-menu-extension.c	\
	ephy-tabs-menu-extension.h	\
	ephy-tab-move-menu.c		\
	ephy-tab-move-menu.h		\
	tabs-menu.c
libtabsmenuextension_la_LDFLAGS = -module -avoid-version

libtabsmenuextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libtabsmenuextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

extensionxmldir = $(extensiondir)
extensionxml_in_files = tabsmenu.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

CLEANFILES = $(extensionxml_DATA)
DISTCLEANFILES = $(extensionxml_DATA)

EXTRA_DIST = $(extensionxml_in_files)
