SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	-I$(top_srcdir)/extensions/select-stylesheet/mozilla	\
	-I$(top_srcdir)/extensions/select-stylesheet		\
	-I$(top_srcdir)/include					\
	$(EPIPHANY_DEPENDENCY_CFLAGS)				\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED					\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED				\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libselectstylesheetextension.la

libselectstylesheetextension_la_SOURCES = \
	ephy-select-stylesheet-extension.c	\
	ephy-select-stylesheet-extension.h	\
	ephy-css-menu.c				\
	ephy-css-menu.h				\
	extension.c

libselectstylesheetextension_la_LIBADD = \
	mozilla/libselectstylesheetmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

extensionxmldir = $(extensiondir)
extensionxml_in_files = select-stylesheet.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(LOCALE_DIR)|" \
	    -e "s|%LIBRARY%|`source $(extension_LTLIBRARIES) && echo $$dlname`|" \
	$< > $@

@INTLTOOL_XML_RULE@

BUILT_SOURCES = $(extensionxml_DATA)

CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(extensionxml_in_files)
