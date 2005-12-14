extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libextensionsmanageruiextension.la

libextensionsmanageruiextension_la_SOURCES = \
	extensions-manager-ui.c \
	extensions-manager-ui.h \
	ephy-extensions-manager-ui-extension.c	\
	ephy-extensions-manager-ui-extension.h	\
	extension.c

libextensionsmanageruiextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libextensionsmanageruiextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libextensionsmanageruiextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

gladedir = $(pkgdatadir)/glade
glade_DATA = extensions-manager-ui.glade

extensioninidir = $(extensiondir)
extensionini_in_files = extensions-manager-ui.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(glade_DATA) $(extensionini_in_files)
