extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libgesturesextension.la

libgesturesextension_la_SOURCES = 	\
	ephy-gesture.c			\
	ephy-gesture.h			\
	ephy-gestures-extension.c	\
	ephy-gestures-extension.h	\
	gestures.c			\
	stroke.c			\
	stroke.h
libgesturesextension_la_LDFLAGS = -module -avoid-version

libgesturesextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libgesturesextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

gesturesdir = $(pkgdatadir)
gestures_DATA =	ephy-gestures.xml

extensionxmldir = $(extensiondir)
extensionxml_in_files = gestures.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`source $(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

CLEANFILES = $(extensionxml_DATA)
DISTCLEANFILES = $(extensionxml_DATA)

EXTRA_DIST = $(gestures_DATA) $(extensionxml_in_files)
