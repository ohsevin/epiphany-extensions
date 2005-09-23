extensiondir = $(EXTENSIONS_DIR)
extension_LTLIBRARIES = libactionsextension.la

libactionsextension_la_SOURCES = \
	ephy-actions-extension-editor-dialog.c		\
	ephy-actions-extension-editor-dialog.h		\
	ephy-actions-extension-properties-dialog.c	\
	ephy-actions-extension-properties-dialog.h	\
	ephy-actions-extension-util.c			\
	ephy-actions-extension-util.h			\
	ephy-actions-extension.c			\
	ephy-actions-extension.h			\
	extension.c

libactionsextension_la_CPPFLAGS = \
	-I$(top_srcdir)/include					\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	$(AM_CPPFLAGS)

libactionsextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

glade_DATA = \
	action-properties.glade	\
	actions-editor.glade
gladedir = $(pkgdatadir)/glade

extensioninidir = $(extensiondir)
extensionini_in_files = actions.ephy-extension.in.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in.in=.ephy-extension)

%.ephy-extension.in: %.ephy-extension.in.in $(extension_LTLIBRARIES)
	sed -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(glade_DATA) $(extensionini_in_files)
