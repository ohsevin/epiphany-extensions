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

libgesturesextension_la_CPPFLAGS = \
        -I$(top_srcdir)/include				\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	$(AM_CPPFLAGS)

libgesturesextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libgesturesextension_la_LDFLAGS = \
	-module -avoid-version \
	-export-symbols $(top_srcdir)/ephy-extension.symbols \
	$(AM_LDFLAGS)
 
gesturesdir = $(pkgdatadir)
gestures_DATA =	ephy-gestures.xml

extensioninidir = $(extensiondir)
extensionini_in_files = gestures.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(gestures_DATA) $(extensionini_in_files)
