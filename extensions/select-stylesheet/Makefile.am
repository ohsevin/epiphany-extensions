SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libselectstylesheetextension.la

libselectstylesheetextension_la_SOURCES = \
	ephy-select-stylesheet-extension.c	\
	ephy-select-stylesheet-extension.h	\
	ephy-css-menu.c				\
	ephy-css-menu.h				\
	extension.c

libselectstylesheetextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/select-stylesheet/mozilla	\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

libselectstylesheetextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libselectstylesheetextension_la_LIBADD = \
	mozilla/libselectstylesheetmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)
libselectstylesheetextension_la_LDFLAGS = -module -avoid-version

extensionxmldir = $(extensiondir)
extensionxml_in_files = select-stylesheet.xml.in.in
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
