SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libgreasemonkeyextension.la

libgreasemonkeyextension_la_SOURCES = \
	ephy-greasemonkey-extension.c	\
	ephy-greasemonkey-extension.h	\
	greasemonkey-script.c		\
	greasemonkey-script.h		\
	extension.c

libgreasemonkeyextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/greasemonkey/mozilla	\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

libgreasemonkeyextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(PCRE_CFLAGS)			\
	$(AM_CFLAGS)

libgreasemonkeyextension_la_LIBADD = \
	mozilla/libgreasemonkeymozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)		\
	$(PCRE_LIBS)
libgreasemonkeyextension_la_LDFLAGS = -module -avoid-version

extensioninidir = $(extensiondir)
extensionini_in_files = greasemonkey.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(extensionini_in_files)
