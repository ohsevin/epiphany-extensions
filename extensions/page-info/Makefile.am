SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	-I$(top_srcdir)/extensions/page-info/mozilla		\
	-I$(top_srcdir)/extensions/page-info			\
	-I$(top_srcdir)/include					\
	$(EPIPHANY_DEPENDENCY_CFLAGS)				\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED					\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED				\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libpageinfoextension.la

libpageinfoextension_la_SOURCES = \
	ephy-page-info-extension.c	\
	ephy-page-info-extension.h	\
	page-info-dialog.c		\
	page-info-dialog.h		\
	extension.c

libpageinfoextension_la_LIBADD = \
	mozilla/libpageinfomozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

glade_DATA = page-info.glade

gladedir = $(pkgdatadir)/glade

EXTRA_DIST = $(glade_DATA)
