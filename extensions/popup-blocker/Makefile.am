SUBDIRS = mozilla

extensiondir = $(libdir)/epiphany/extensions

INCLUDES = \
        -I$(top_srcdir)/include					\
	-I$(top_srcdir)/extensions/popup-blocker/mozilla	\
	$(EPIPHANY_DEPENDENCY_CFLAGS)				\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED					\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED				\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libpopupblockerextension.la

libpopupblockerextension_la_SOURCES = \
	ephy-popup-blocker-extension.c	\
	ephy-popup-blocker-extension.h	\
	ephy-popup-blocker-icon.c	\
	ephy-popup-blocker-icon.h	\
	ephy-popup-blocker-popup.c	\
	ephy-popup-blocker-popup.h	\
	popup-blocker.c

libpopupblockerextension_la_LIBADD = \
	mozilla/libpopupblockermozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

icon_DATA = popup-blocker.svg

icondir = $(pkgdatadir)/icons

EXTRA_DIST = $(icon_DATA)
