SUBDIRS = mozilla

extensiondir = $(EXTENSIONS_DIR)

INCLUDES = \
	-I$(top_srcdir)/extensions/sample-mozilla/mozilla	\
        -I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"	\
	-DEPHY_PLUGINS_LOCALEDIR=\"$(datadir)/locale\"  \
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libsamplemozillaextension.la

libsamplemozillaextension_la_SOURCES = \
	ephy-sample2-extension.c	\
	ephy-sample2-extension.h	\
	sample-mozilla.c

libsamplemozillaextension_la_LIBADD = \
	mozilla/libsamplemozillamozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)
