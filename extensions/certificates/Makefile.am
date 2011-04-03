SUBDIRS =

extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libcertificatesextension.la

libcertificatesextension_la_SOURCES = \
	ephy-certificates-extension.c	\
	ephy-certificates-extension.h

libcertificatesextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libcertificatesextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libcertificatesextension_la_LDFLAGS = \
	-module -avoid-version \
	-no-undefined \
	$(AM_LDFLAGS)

extensioninidir = $(extensiondir)
extensionini_in_files = certificates.plugin.in.in
extensionini_DATA = $(extensionini_in_files:.plugin.in.in=.plugin)

%.plugin.in: %.plugin.in.in $(extension_LTLIBRARIES)
	$(AM_V_GEN) \
	sed -e "s|%LIBRARY%|`echo $(extension_LTLIBRARIES) | cut -d. -f1`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(extensionini_in_files)
