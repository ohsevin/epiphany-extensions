extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libfindextension.la

libfindextension_la_SOURCES = \
	eggfindbar.c		\
	eggfindbar.h		\
	eggfindbarprivate.h	\
	ephy-find-extension.c	\
	ephy-find-extension.h	\
	ephy-find-bar.cpp	\
	ephy-find-bar.h		\
	mozilla-helpers.cpp	\
	mozilla-helpers.h	\
	TypeAheadFind.cpp	\
	TypeAheadFind.h		\
	extension.c

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	fastfind	\
	find		\
	xpcom

libfindextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include					\
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DALLOW_PRIVATE_API		\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  	\
	$(AM_CPPFLAGS)

libfindextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libfindextension_la_CXXFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(MOZILLA_COMPONENT_CFLAGS)	\
	$(AM_CXXFLAGS)

libfindextension_la_LIBADD = \
	$(MOZILLA_COMPONENT_LIBS)

extensionxmldir = $(extensiondir)
extensionxml_in_files = find.xml.in.in
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

EXTRA_DIST = $(extensionxml_in_files)
