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
	ephy-actions-extension.h

libactionsextension_la_CPPFLAGS = \
	-I$(top_srcdir)/include					\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	$(AM_CPPFLAGS)

libactionsextension_la_CFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

libactionsextension_la_LDFLAGS = \
	-module -avoid-version \
	-no-undefined \
	$(AM_LDFLAGS)

ui_DATA = \
	action-properties.ui	\
	actions-editor.ui
uidir = $(pkgdatadir)/ui

extensioninidir = $(extensiondir)
extensionini_in_files = actions.plugin.in.in
extensionini_DATA = $(extensionini_in_files:.plugin.in.in=.plugin)

%.plugin.in: %.plugin.in.in $(extension_LTLIBRARIES)
	$(AM_V_GEN) \
	sed -e "s|%LIBRARY%|`echo $(extension_LTLIBRARIES) | cut -d. -f1`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@EPIPHANY_EXTENSION_RULE@

CLEANFILES = $(extensionini_DATA)
DISTCLEANFILES = $(extensionini_DATA)

EXTRA_DIST = $(ui_DATA) $(extensionini_in_files)
