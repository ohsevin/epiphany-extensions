extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libtabsreloadedextension.la

libtabsreloadedextension_la_SOURCES = \
	ephy-cell-renderer-tab.c \
	ephy-cell-renderer-tab.h \
	ephy-tabs-manager.c \
	ephy-tabs-manager.h \
	ephy-tabs-reloaded-extension.c \
	ephy-tabs-reloaded-extension.h \
	tabs-reloaded.c

libtabsreloadedextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include	\
	-DSHARE_DIR=\"$(pkgdatadir)\" \
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\" \
	$(AM_CPPFLAGS)

libtabsreloadedextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS) \
	$(AM_CFLAGS)

libtabsreloadedextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

gladedir = $(pkgdatadir)/glade
glade_DATA = \
	tabs-reloaded.ui

extensioninidir = $(extensiondir)
extensionini_in_files = tabs-reloaded.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(extensionini_in_files)
