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

libsidebarextension_la_LDFLAGS = -module -avoid-version

uimanager_DATA = epiphany-sidebar-ui.xml

uimanagerdir = $(pkgdatadir)/xml

extensioninidir = $(extensiondir)
extensionini_in_files = sidebar.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(BUILT_SOURCES) $(extensionini_DATA)
DISTCLEANFILES = $(BUILT_SOURCES) $(extensionini_DATA)

EXTRA_DIST = \
	$(uimanager_DATA)		\
	$(extensionini_in_files)

