extensiondir = $(libdir)/epiphany/extensions

INCLUDES = \
	-I$(top_srcdir)/include 			\
	$(EPIPHANY_DEPENDENCY_CFLAGS)			\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DEPHY_PLUGINS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED			\
	-DGTK_DISABLE_DEPRECATED			\
	-DGDK_PIXBUF_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libtabsmenuextension.la

libtabsmenuextension_la_SOURCES = \
	ephy-tabs-menu-extension.c	\
	ephy-tabs-menu-extension.h	\
	ephy-tab-move-menu.c		\
	ephy-tab-move-menu.h		\
	tabs-menu.c
