SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	-I$(top_srcdir)/extensions/adblock/mozilla	\
        -I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DEPHY_PLUGINS_LOCALEDIR=\"$(datadir)/locale\"  \
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED	\
	-D_GNU_SOURCE

extension_LTLIBRARIES = libadblockextension.la

libadblockextension_la_SOURCES = \
	ephy-adblock-extension.c	\
	ephy-adblock-extension.h	\
	ad-blocker.h			\
	ad-blocker.c			\
	extension.c

libadblockextension_la_LIBADD = \
	mozilla/libadblockmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

patterns_DATA = adblock-patterns
patternsdir = $(pkgdatadir)

EXTRA_DIST = $(patterns_DATA)
