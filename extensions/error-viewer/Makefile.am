SUBDIRS = mozilla

if HAVE_OPENSP
SUBDIRS += opensp
endif

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = liberrorviewerextension.la

liberrorviewerextension_la_SOURCES = \
	ephy-error-viewer-extension.c	\
	ephy-error-viewer-extension.h	\
	error-viewer.c			\
	error-viewer.h			\
	link-checker.c			\
	link-checker.h			\
	extension.c

if HAVE_OPENSP
liberrorviewerextension_la_SOURCES += \
	 sgml-validator.c	\
	 sgml-validator.h
endif

liberrorviewerextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-I$(top_srcdir)/extensions/error-viewer/mozilla	\
	-I$(top_srcdir)/extensions/error-viewer/opensp	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

if HAVE_OPENSP
liberrorviewerextension_la_CPPFLAGS += \
	-DHAVE_OPENSP
endif

liberrorviewerextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

liberrorviewerextension_la_LIBADD = \
	mozilla/liberrorviewermozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

if HAVE_OPENSP
liberrorviewerextension_la_LIBADD += \
	opensp/liberrorvieweropensp.la	\
	-losp
endif
liberrorviewerextension_la_LDFLAGS = -module -avoid-version

glade_DATA = error-viewer.glade

gladedir = $(pkgdatadir)/glade

extensionxmldir = $(extensiondir)
extensionxml_in_files = error-viewer.xml.in.in
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

EXTRA_DIST = $(glade_DATA) $(extensionxml_in_files)
