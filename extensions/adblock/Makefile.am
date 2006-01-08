extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libadblockextension.la

libadblockextension_la_SOURCES = \
	ephy-adblock-extension.c	\
	ephy-adblock-extension.h	\
	ad-blocker.h			\
	ad-blocker.c			\
	ad-uri-tester.h			\
	ad-uri-tester.c			\
	extension.c

libadblockextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	-D_GNU_SOURCE					\
	$(AM_CPPFLAGS)

libadblockextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(PCRE_CFLAGS)			\
	$(AM_CFLAGS)

libadblockextension_la_LIBADD = \
	$(PCRE_LIBS)

libadblockextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

patterns_DATA = adblock-patterns
patternsdir = $(pkgdatadir)

update-adblock-patterns: download-latest.pl
	perl $< > $(patterns_DATA)

icon_DATA = adblock-statusbar-icon.svg
icondir = $(pkgdatadir)

extensioninidir = $(extensiondir)
extensionini_in_files = adblock.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(patterns_DATA) $(icon_DATA) $(extensionini_in_files)
