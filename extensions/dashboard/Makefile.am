extensiondir = $(EXTENSIONS_DIR)

INCLUDES =	\
   	-I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"pkgdatadir\"	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libdashboardextension.la

libdashboardextension_la_SOURCES =	\
	ephy-dashboard.c		\
     	ephy-dashboard-extension.c	\
	ephy-dashboard-extension.h	\
	dashboard-frontend-xmlwriter.h	\
	dashboard-frontend-xmlwriter.c

extensionxmldir = $(extensiondir)
extensionxml_in_files = dashboard.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(LOCALE_DIR)|" \
	    -e "s|%LIBRARY%|FIXME|" \
	$< > $@

@INTLTOOL_XML_RULE@

BUILT_SOURCES = $(extensionxml_DATA)

DISTCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(extensionxml_in_files)
