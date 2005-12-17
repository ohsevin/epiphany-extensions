extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libimagebarextension.la

libimagebarextension_la_SOURCES = \
	ephy-imagebar-extension.c	\
	ephy-imagebar-extension.h	\
	mozilla-event.cpp		\
	mozilla-event.h			\
	extension.c
libimagebarextension_la_LDFLAGS = -module -avoid-version

libimagebarextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(EPIPHANY_DEPENDENCY_CFLAGS)			\
	$(MOZILLA_COMPONENT_CFLAGS)			\
	$(AM_CPPFLAGS)

libimagebarextension_la_CFLAGS = \
	-I$(MOZILLA_INCLUDE_ROOT)	\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

extensioninidir = $(extensiondir)
extensionini_in_files = imagebar.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(extensionini_in_files)
