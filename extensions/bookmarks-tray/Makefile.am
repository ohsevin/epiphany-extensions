extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libbookmarkstrayextension.la

libbookmarkstrayextension_la_SOURCES = \
	ephy-bookmarks-tray-extension.c	\
	ephy-bookmarks-tray-extension.h	\
	extension.c

libbookmarkstrayextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

libbookmarkstrayextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libbookmarkstrayextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

ui_xmldir = $(pkgdatadir)/xml
ui_xml_DATA = ephy-bookmarks-tray-ui.xml

extensioninidir = $(extensiondir)
extensionini_in_files = bookmarks-tray.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = \
	$(ui_xml_DATA)			\
	$(extensionini_in_files)
