SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	$(WARN_CFLAGS) \
	-I$(top_srcdir)/extensions/sidebar/mozilla	\
	-I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libsidebarextension.la

nodist_libsidebarextension_la_SOURCES =	\
	ephy-sidebar-marshal.c		\
	ephy-sidebar-marshal.h

libsidebarextension_la_SOURCES = \
	ephy-sidebar-extension.c	\
	ephy-sidebar-extension.h	\
	ephy-sidebar.c			\
	ephy-sidebar.h			\
	ephy-sidebar-embed.c		\
	ephy-sidebar-embed.h		\
	sidebar-commands.c		\
	sidebar-commands.h		\
	extension.c

libsidebarextension_la_LIBADD = \
	mozilla/libsidebarmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

BUILT_SOURCES = \
	ephy-sidebar-marshal.c	\
	ephy-sidebar-marshal.h

CLEANFILES = \
	$(BUILT_SOURCES)

ephy-sidebar-marshal.c: ephy-sidebar-marshal.list
	$(GLIB_GENMARSHAL) --prefix=ephy_sidebar_marshal $(srcdir)/ephy-sidebar-marshal.list --header --body >  ephy-sidebar-marshal.c

ephy-sidebar-marshal.h: ephy-sidebar-marshal.list
	$(GLIB_GENMARSHAL) --prefix=ephy_sidebar_marshal $(srcdir)/ephy-sidebar-marshal.list --header > ephy-sidebar-marshal.h


uimanager_DATA = epiphany-sidebar-ui.xml

uimanagerdir = $(pkgdatadir)/xml

EXTRA_DIST = \
	ephy-sidebar-marshal.list	\
	$(uimanager_DATA)
