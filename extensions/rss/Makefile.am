extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = librssextension.la

librssextension_la_SOURCES = \
	ephy-rss-extension.c	\
	ephy-rss-extension.h	\
	rss-ui.c                \
	rss-ui.h                \
	rss-feedlist.c			\
	rss-feedlist.h			\
	extension.c
librssextension_la_CPPFLAGS = \
    -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

librssextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(DBUS_CFLAGS) \
	$(AM_CFLAGS)

librssextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

uidir = $(pkgdatadir)/ui
ui_DATA = rss-ui.ui

extensioninidir = $(extensiondir)
extensionini_in_files = rss.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = \
	$(extensionini_in_files)	\
	$(ui_DATA)
