plugindir = $(libdir)/epiphany/plugins

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

plugin_LTLIBRARIES = libtabsplugin.la

libtabsplugin_la_SOURCES = \
	ephy-tabs-plugin.c		\
	ephy-tabs-plugin-menu.c		\
	ephy-tabs-plugin-menu.h		\
	ephy-window-action.c		\
	ephy-window-action.h


EXTRA_DIST = epiphany.diff
