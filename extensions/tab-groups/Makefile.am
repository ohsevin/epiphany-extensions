extensiondir = $(EXTENSIONS_DIR)

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

extension_LTLIBRARIES = libtabgroupsextension.la

libtabgroupsextension_la_SOURCES = \
	ephy-tab-groups-extension.c	\
	ephy-tab-groups-extension.h	\
	ephy-tab-grouper.c		\
	ephy-tab-grouper.h		\
	tab-groups.c
