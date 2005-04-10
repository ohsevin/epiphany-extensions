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
librssextension_la_LDFLAGS = -module -avoid-version

librssextension_la_CPPFLAGS = \
    -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

librssextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(DBUS_CFLAGS) \
	$(AM_CFLAGS)

gladedir = $(pkgdatadir)/glade
glade_DATA = rss-ui.glade

extensionxmldir = $(extensiondir)
extensionxml_in_files = rss.xml.in.in
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
