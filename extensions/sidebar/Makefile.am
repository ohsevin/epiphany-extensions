extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libsidebarextension.la

libsidebarextension_la_SOURCES = \
	ephy-sidebar-extension.c	\
	ephy-sidebar-extension.h	\
	ephy-sidebar.c			\
	ephy-sidebar.h			\
	ephy-sidebar-embed.c		\
	ephy-sidebar-embed.h		\
	sidebar-commands.c		\
	sidebar-commands.h		\
	extension.c

libsidebarextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libsidebarextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

uimanager_DATA = epiphany-sidebar-ui.xml

uimanagerdir = $(pkgdatadir)/xml

extensionxmldir = $(extensiondir)
extensionxml_in_files = sidebar.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`source $(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

CLEANFILES = $(BUILT_SOURCES) $(extensionxml_DATA)
DISTCLEANFILES = $(BUILT_SOURCES) $(extensionxml_DATA)

EXTRA_DIST = \
	$(uimanager_DATA)		\
	$(extensionxml_in_files)

