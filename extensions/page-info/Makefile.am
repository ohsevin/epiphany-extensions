SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libpageinfoextension.la

libpageinfoextension_la_SOURCES = \
	ephy-page-info-extension.c	\
	ephy-page-info-extension.h	\
	page-info-dialog.c		\
	page-info-dialog.h		\
	extension.c

libpageinfoextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-I$(top_srcdir)/extensions/page-info/mozilla	\
	-I$(MOZILLA_INCLUDE_ROOT)/gtkembedmoz		\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libpageinfoextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libpageinfoextension_la_LIBADD = \
	mozilla/libpageinfomozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)
libpageinfoextension_la_LDFLAGS = -module -avoid-version

glade_DATA = page-info.glade
gladedir = $(pkgdatadir)/glade

ui_xml_DATA = page-info-context-ui.xml
ui_xmldir = $(pkgdatadir)/xml

extensioninidir = $(extensiondir)
extensionini_in_files = page-info.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(glade_DATA) $(ui_xml_DATA) $(extensionini_in_files)
