extensiondir = $(libdir)/epiphany/extensions

INCLUDES = \
        -I$(top_srcdir)/include					\
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
	popup-blocker.c
