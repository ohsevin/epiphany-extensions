extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libadblockextension.la

libadblockextension_la_SOURCES = \
	ephy-adblock-extension.c	\
	ephy-adblock-extension.h	\
	uri-tester.h			\
	uri-tester.c			\
	adblock-ui.h			\
	adblock-ui.c			\
	extension.c

libadblockextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	-D_GNU_SOURCE					\
	$(AM_CPPFLAGS)

libadblockextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libadblockextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

uidir = $(pkgdatadir)/ui
ui_DATA = adblock.ui

extensioninidir = $(extensiondir)
extensionini_in_files = adblock.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	$(AM_V_GEN) \
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(ui_DATA) $(extensionini_in_files)
