extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libimagebarextension.la

libimagebarextension_la_SOURCES = \
	ephy-imagebar-extension.c	\
	ephy-imagebar-extension.h	\
	mozilla-event.cpp		\
	mozilla-event.h			\
	extension.c

gecko_include_subdirs = \
	.		\
	dom		\
	webbrwsr

libimagebarextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include		\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(gecko_include_subdirs)) \
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libimagebarextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(AM_CFLAGS)

libimagebarextension_la_CXXFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(AM_CXXFLAGS)

libimagebarextension_la_LIBADD = \
	$(MOZILLA_COMPONENT_LIBS)

libimagebarextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)

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
