SUBDIRS = mozilla

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
	-I$(top_srcdir)/extensions/adblock/mozilla	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	-D_GNU_SOURCE					\
	$(AM_CPPFLAGS)

libadblockextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(PCRE_CFLAGS)			\
	$(AM_CFLAGS)

libadblockextension_la_LIBADD = \
	mozilla/libadblockmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)	\
	$(PCRE_LIBS)

libadblockextension_la_LDFLAGS = -module -avoid-version

patterns_DATA = adblock-patterns
patternsdir = $(pkgdatadir)

update-adblock-patterns: download-latest.pl
	perl $< > $(patterns_DATA)

icon_DATA = adblock-statusbar-icon.svg
icondir = $(pkgdatadir)

extensionxmldir = $(extensiondir)
extensionxml_in_files = adblock.xml.in.in
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

EXTRA_DIST = $(patterns_DATA) $(icon_DATA) $(extensionxml_in_files)
