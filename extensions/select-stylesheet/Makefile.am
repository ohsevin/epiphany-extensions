SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	-I$(top_srcdir)/extensions/select-stylesheet/mozilla	\
	-I$(top_srcdir)/extensions/select-stylesheet		\
	-I$(top_srcdir)/include					\
	$(EPIPHANY_DEPENDENCY_CFLAGS)				\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED					\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED				\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libselectstylesheetextension.la

libselectstylesheetextension_la_SOURCES = \
	ephy-select-stylesheet-extension.c	\
	ephy-select-stylesheet-extension.h	\
	ephy-css-menu.c				\
	ephy-css-menu.h				\
	extension.c

libselectstylesheetextension_la_LIBADD = \
	mozilla/libselectstylesheetmozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)
