SUBDIRS =

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libcertificatesextension.la

libcertificatesextension_la_SOURCES = \
	ephy-certificates-extension.c	\
	ephy-certificates-extension.h	\
	extension.c
libcertificatesextension_la_LDFLAGS = -module -avoid-version

libcertificatesextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libcertificatesextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

extensionxmldir = $(extensiondir)
extensionxml_in_files = certificates.xml.in.in
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
