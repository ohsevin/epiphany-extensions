extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libtoolbarextrasextension.la

libtoolbarextrasextension_la_SOURCES = \
	ephy-toolbar-extras-extension.h	\
	ephy-toolbar-extras-extension.c	\
	ephy-multi-smart-action.h	\
	ephy-multi-smart-action.c	\
	galago-gtk-icon-entry.c		\
	galago-gtk-icon-entry.h		\
	toolbar-extras.c

libtoolbarextrasextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libtoolbarextrasextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libtoolbarextrasextension_la_LDFLAGS = -module -avoid-version

extensionxmldir = $(extensiondir)
extensionxml_in_files = toolbar-extras.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`source $(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

CLEANFILES = $(extensionxml_DATA)
DISTCLEANFILES = $(extensionxml_DATA)

EXTRA_DIST = $(extensionxml_in_files)
