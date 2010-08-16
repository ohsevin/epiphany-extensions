extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libautoreloadextension.la

libautoreloadextension_la_SOURCES = \
	ephy-auto-reload-extension.c	\
	ephy-auto-reload-extension.h	\
	extension.c
	
libautoreloadextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

libautoreloadextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libautoreloadextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

extensioninidir = $(extensiondir)
extensionini_in_files = auto-reload.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	$(AM_V_GEN) \
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(extensionini_in_files)
